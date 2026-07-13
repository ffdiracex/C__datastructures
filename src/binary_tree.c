/*   COPYRIGHT (C) ffdiracex 2026
 *
 *     Binary Search Tree implementation in C
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
    int key;
    struct node *left;
    struct node *right;
    int height;
};

static struct node *
create_node(int key)
{
    struct node *new_node = (struct node*)malloc(sizeof(struct node));
    if (!new_node) {
        fprintf(stderr, "memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    new_node->key = key;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 1;
    return new_node;
}

static int
height(struct node *n)
{
    return n ? n->height : 0;
}

static int
balance_factor(struct node *n)
{
    return n ? height(n->left) - height(n->right) : 0;
}

static void
update_height(struct node *n)
{
    if (n) {
        int l = height(n->left);
        int r = height(n->right);
        n->height = (l > r ? l : r) + 1;
    }
}

static struct node *
rotate_right(struct node *y)
{
    struct node *x = y->left;
    struct node *t2 = x->right;
    x->right = y;
    y->left = t2;
    update_height(y);
    update_height(x);
    return x;
}

static struct node *
rotate_left(struct node *x)
{
    struct node *y = x->right;
    struct node *t2 = y->left;
    y->left = x;
    x->right = t2;
    update_height(x);
    update_height(y);
    return y;
}

static struct node *
insert(struct node *root, int key)
{
    if (!root) return create_node(key);

    if (key < root->key)
        root->left = insert(root->left, key);
    else if (key > root->key)
        root->right = insert(root->right, key);
    else
        return root;

    update_height(root);
    int balance = balance_factor(root);

    if (balance > 1 && key < root->left->key)
        return rotate_right(root);
    if (balance < -1 && key > root->right->key)
        return rotate_left(root);
    if (balance > 1 && key > root->left->key) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance < -1 && key < root->right->key) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}

static struct node *
min_value_node(struct node *root)
{
    struct node *current = root;
    while (current && current->left)
        current = current->left;
    return current;
}

static struct node *
delete_node(struct node *root, int key)
{
    if (!root) return root;

    if (key < root->key)
        root->left = delete_node(root->left, key);
    else if (key > root->key)
        root->right = delete_node(root->right, key);
    else {
        if (!root->left || !root->right) {
            struct node *temp = root->left ? root->left : root->right;
            if (!temp) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            struct node *temp = min_value_node(root->right);
            root->key = temp->key;
            root->right = delete_node(root->right, temp->key);
        }
    }

    if (!root) return root;

    update_height(root);
    int balance = balance_factor(root);

    if (balance > 1 && balance_factor(root->left) >= 0)
        return rotate_right(root);
    if (balance > 1 && balance_factor(root->left) < 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance < -1 && balance_factor(root->right) <= 0)
        return rotate_left(root);
    if (balance < -1 && balance_factor(root->right) > 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}

static bool
search(struct node *root, int key)
{
    if (!root) return false;
    if (key == root->key) return true;
    return key < root->key ? search(root->left, key) : search(root->right, key);
}

static void
inorder(struct node *root)
{
    if (root) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

static void
preorder(struct node *root)
{
    if (root) {
        printf("%d ", root->key);
        preorder(root->left);
        preorder(root->right);
    }
}

static void
postorder(struct node *root)
{
    if (root) {
        postorder(root->left);
        postorder(root->right);
        printf("%d ", root->key);
    }
}

static void
print_tree(struct node *root, int space, int indent)
{
    if (!root) return;
    space += indent;
    print_tree(root->right, space, indent);
    printf("\n");
    for (int i = indent; i < space; i++) printf(" ");
    printf("%d\n", root->key);
    print_tree(root->left, space, indent);
}

static int
count_nodes(struct node *root)
{
    return root ? 1 + count_nodes(root->left) + count_nodes(root->right) : 0;
}

static int
max_depth(struct node *root)
{
    if (!root) return 0;
    int l = max_depth(root->left);
    int r = max_depth(root->right);
    return (l > r ? l : r) + 1;
}

static bool
is_balanced(struct node *root)
{
    if (!root) return true;
    int b = balance_factor(root);
    return abs(b) <= 1 && is_balanced(root->left) && is_balanced(root->right);
}

static void
free_tree(struct node *root)
{
    if (root) {
        free_tree(root->left);
        free_tree(root->right);
        free(root);
    }
}

/* DATA-DRIVEN TESTING*/

typedef struct {
    enum { OP_INSERT, OP_DELETE, OP_SEARCH, OP_SHOW, OP_STATS } type;
    int key;
    const char *label;
} operation;

#define INSERT(k)    { OP_INSERT, k, "insert" }
#define DELETE(k)    { OP_DELETE, k, "delete" }
#define SEARCH(k)    { OP_SEARCH, k, "search" }
#define SHOW         { OP_SHOW, 0, "show" }
#define STATS        { OP_STATS, 0, "stats" }

static void
run_operations(struct node **tree, operation ops[], int count)
{
    for (int i = 0; i < count; i++) {
        switch (ops[i].type) {
            case OP_INSERT:
                *tree = insert(*tree, ops[i].key);
                printf("%s %d: ", ops[i].label, ops[i].key);
                inorder(*tree);
                printf("\n");
                break;
                
            case OP_DELETE:
                *tree = delete_node(*tree, ops[i].key);
                printf("%s %d: ", ops[i].label, ops[i].key);
                inorder(*tree);
                printf("\n");
                break;
                
            case OP_SEARCH:
                printf("%s %d: %s\n", ops[i].label, ops[i].key,
                       search(*tree, ops[i].key) ? "FOUND" : "NOT FOUND");
                break;
                
            case OP_SHOW:
                printf("\nTree:\n");
                print_tree(*tree, 0, 4);
                break;
                
            case OP_STATS:
                printf("Nodes: %d, Depth: %d, Balanced: %s\n",
                       count_nodes(*tree),
                       max_depth(*tree),
                       is_balanced(*tree) ? "Yes" : "No");
                break;
        }
    }
}

/* STRING-BASED COMMANDS */

static void
run_commands(struct node **tree, const char *commands[], int count)
{
    for (int i = 0; i < count; i++) {
        const char *cmd = commands[i];
        char op[10];
        int key;
        
        if (sscanf(cmd, "%s %d", op, &key) == 2) {
            if (strcmp(op, "insert") == 0) {
                *tree = insert(*tree, key);
                printf("%s %d: ", op, key);
                inorder(*tree);
                printf("\n");
            } else if (strcmp(op, "delete") == 0) {
                *tree = delete_node(*tree, key);
                printf("%s %d: ", op, key);
                inorder(*tree);
                printf("\n");
            } else if (strcmp(op, "search") == 0) {
                printf("%s %d: %s\n", op, key,
                       search(*tree, key) ? "FOUND" : "NOT FOUND");
            }
        } else if (strcmp(cmd, "show") == 0) {
            printf("\nTree:\n");
            print_tree(*tree, 0, 4);
        } else if (strcmp(cmd, "stats") == 0) {
            printf("Nodes: %d, Depth: %d, Balanced: %s\n",
                   count_nodes(*tree),
                   max_depth(*tree),
                   is_balanced(*tree) ? "Yes" : "No");
        } else if (strcmp(cmd, "clear") == 0) {
            free_tree(*tree);
            *tree = NULL;
            printf("Tree cleared\n");
        }
    }
}

/* ----- MAIN ----- */

int
main(void)
{
    struct node *tree = NULL;

    operation ops[] = {
        INSERT(50), INSERT(30), INSERT(70), INSERT(20), INSERT(40),
        INSERT(60), INSERT(80), INSERT(15), INSERT(25), INSERT(35),
        INSERT(45), INSERT(55), INSERT(65), INSERT(75), INSERT(85),
        SHOW, STATS,
        SEARCH(40), SEARCH(100), SEARCH(25),
        DELETE(50), SHOW, STATS,
        DELETE(20), SHOW, STATS,
        DELETE(30), SHOW, STATS,
        DELETE(70), SHOW, STATS,
        SHOW,
        // Start fresh
        INSERT(10), INSERT(20), INSERT(30), INSERT(40), INSERT(50),
        INSERT(60), INSERT(70), SHOW, STATS
    };

    run_operations(&tree, ops, sizeof(ops)/sizeof(ops[0]));
    free_tree(tree);

    printf("\n--- Using string commands ---\n\n");
    tree = NULL;
    
    const char *commands[] = {
        "insert 5", "insert 3", "insert 7", "insert 2", "insert 4",
        "insert 6", "insert 8", "show", "stats",
        "search 4", "search 9",
        "delete 5", "show", "stats",
        "clear",
        "insert 100", "insert 200", "insert 150", "show"
    };

    run_commands(&tree, commands, sizeof(commands)/sizeof(commands[0]));
    free_tree(tree);

    printf("\n--- Batch operations from raw data ---\n\n");
    tree = NULL;
    
    int batch_insert[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int batch_delete[] = {5, 2, 8};
    int batch_search[] = {3, 7, 11};

    printf("Inserting batch: ");
    for (int i = 0; i < sizeof(batch_insert)/sizeof(int); i++) {
        tree = insert(tree, batch_insert[i]);
        printf("%d ", batch_insert[i]);
    }
    printf("\nInorder: ");
    inorder(tree);
    printf("\n\n");

    printf("Deleting batch: ");
    for (int i = 0; i < sizeof(batch_delete)/sizeof(int); i++) {
        tree = delete_node(tree, batch_delete[i]);
        printf("%d ", batch_delete[i]);
    }
    printf("\nInorder: ");
    inorder(tree);
    printf("\n\n");

    printf("Searching batch: ");
    for (int i = 0; i < sizeof(batch_search)/sizeof(int); i++) {
        printf("%d:%s ", batch_search[i],
               search(tree, batch_search[i]) ? "YES" : "NO");
    }
    printf("\n");

    print_tree(tree, 0, 4);
    free_tree(tree);

    return 0;
}
