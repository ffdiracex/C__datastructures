//minimal "dir" command

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>

int main(int argc, char *argv[])
{
    char buf[4096];
    int fd = open(argc < 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    if (fd < 0) return 1;

    struct dirent *d;
    long n;
    char *p;

    while ((n=SYS_getdents64((int)fd, buf, 4096)) > 0)
    {
        p = buf;
        while (p < buf + n)
        {
            d = (struct dirent *)p;
            if (d->d_name[0] != '.')
            {
                write(1, d->d_name, strlen(d->d_name));
                write(1, " ", 2);
            }
            p += d->d_reclen;
        }
    }
    write(1, "\n", 1);
    close(fd);
    return 0;
}


