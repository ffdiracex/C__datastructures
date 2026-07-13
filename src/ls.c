// impl of 'ls' util, attempting to be a little simpler and portable.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <sys/types.h>
#include <termios.h>
#include <setjmp.h>
#include <getopt.h>
#include <signal.h>
#include <fnmatch.h>
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#define PROGRAM_NAME (ls_mode == LS_LS ? "ls" \
        : (ls_mode == LS_MULTI_COL \
            ? "dir" : "vdir"))

#define CREDITS "Credits go to the GNU authors, i.e. Stallman"
typedef ptrdiff_t idx_t;
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

enum filetype
{
    unknown,
    fifo,
    chardev,
    directory,
    blockdev,
    normal,
    symbolic_link,
    whiteout,
    sock,
    arg_directory
};

enum { filetype_cardinality = arg_directory + 1 };

/* what can the file types be represented as? if we look at the tool 'find' we can use 'find -type d' or 'find -type f' which is directory and file, respectively/ */
static char const filetype_letter[] = {'?', 'p', 'c', 'd', 'b', '-', 'l', 's', 'w', 'd'};

/* MAP enum filetype to d_type values from  <dirent.h> */
static unsigned char const filetype_d_type[] =
{
    DT_UNKNOWN, DT_FIFO, DT_CHR, DT_DIR, DT_BLK, DT_REG, DT_LNK, DT_SOCK, DT_WHT, DT_DIR
};

/* MAP d_type values to enum filetype */
static char const d_type_filetype[UCHAR_MAX + 1] =
{
    [DT_BLK] = blockdev, [DT_CHR] = chardev, [DT_DIR] = directory,
    [DT_FIFO] = fifo, [DT_LNK] = symbolic_link, [DT_REG] = normal,
    [DT_SOCK] = sock, [DT_WHT] = whiteout
};

enum acl_type
{
    ACL_T_NONE,
    ACL_T_UNKNOWN,
    ACL_T_LSM_CONTEXT_ONLY,
    ACL_T_YES
};

struct fileinfo
{
    /* FILENAME */
    char *name;

    /* symbolic link, i.e. the name of the file it is linked to. for example: ls -> /usr/bin/ls, or something along the lines */
    char *linkname;

    /* terminal hyperlinks */
    char *absolute_name;
    struct stat stat;
    
    enum filetype filetype;

    /* For symbolic link and long listing, st_mode of file linked to, else 0 */
    mode_t linkmode;
    
    //security
    char *scontext;

    bool stat_ok; //OK status

    /* if linked file exists, flag it with OK */
    bool linkok;

    /* if linked file has an access specifier (specific owners, or some security related shenanigans) */
    enum acl_type acl_type;

    /* if file has capability information, enable color listing. */
    bool has_capability;

    /* does filename require quotes, i.e. has spaces or similar */
    int quoted;

    /* cached screen width */
    size_t width;
};

/* NULL is a valid character in a color indicator, so we have to use a string type (i.e. null terminator etc. ) */
struct bin_str
{
    size_t len;
    char const *string;
};

static size_t quote_name (char const *name, struct quoting_options const *options, int needs_general_quoting,
    const struct bin_str *color, bool allow_pad, struct obstack *stack,
    char const *absolute_name);

static size_t quote_name_buf (char **inbuf, size_t bufsize, char *name, struct quoting_options const *options,
        int needs_general_quoting, size_t *width, bool *pad);

static int decode_switches (int argc, char **argv);
static bool file_ignore (char const *name);
static uintmax_t gobble_file (char const *name, enum filetype type,
        ino_t inode, bool command_line_arg, char const *dirname);
static const struct bin_str *get_color_indicator (const struct fileinfo *f, bool symlink_target);
static bool print_color_indicator (const struct bin_str *ind);
static void put_indicator (const struct bin_str *ind);
static void add_ignore_pattern (char const *pattern);
static void attach (char *dest, char const *dirname, char const *name);
static void clear_files (void);
static void extract_dirs_from_files (char const *dirname, bool cmd_line_arg);
static void get_link_name (char const *filename, struct fileinfo *f, bool cmd_line_arg);
static void indent (size_t from, size_t to);
static idx_t calculate_columns (bool by_columns);
static void print_current_files (void);
static void print_dir (char const *name, char const *realname, bool cmd_line_arg);
static size_t print_file_name_and_frills (const struct fileinfo *f, size_t start_col);
static void print_horizontal (void);
static int format_user_width (uid_t u);
static int format_group_width (gid_t g);
static void print_long_format (const struct fileinfo *f);
static void print_many_per_line (void);
static size_t print_name_with_quoting (const struct fileinfo *f,
            bool symlink_target, struct obstack *stack, size_t start_col);
static void prep_non_filename_text (void);
static bool print_type_indicator (bool stat_ok, mode_t mode,
            enum filetype type);
static void print_with_separator (char sep);
static void queue_directory (char const *name, char const *realname, bool cmd_line_arg);
static void sort_files (void);
static void parse_ls_color (void);
static int getenv_quoting_style (void);

static size_t quote_name_width (char const *name,
            struct quoting_options const *options,
            int needs_general_quoting);

/* Initial size of hash table. most are likely to be smaller/shallower than this */
enum { INITIAL_TABLE_SIZE = 31 };

static Hash_table *active_dir_set;

// to be continued
