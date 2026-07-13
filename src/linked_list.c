/*   COPYRIGHT (C) ffdiracex 2026
 *
 *      Linked List implementation in C
 * */

#include <stdio.h>
#include <stdlib.h>

// NODE
//  @ represent the Node Structure of the Linked List
struct Node {
    int data;
    struct Node *next;
};

//MACROS
// These macros make testing cleaner and more readable

// Print with a descriptive message
#define PRINT_MSG(list, msg) \
    do { \
        printf("%s\n", msg); \
        print(list); \
    } while(0)

// Insert at first and print
#define INSERT_FIRST_AND_PRINT(head, val, msg) \
    do { \
        printf("%s: %d\n", msg, val); \
        insert_first(head, val); \
        print(*head); \
    } while(0)

// Insert at last and print
#define INSERT_LAST_AND_PRINT(head, val, msg) \
    do { \
        printf("%s: %d\n", msg, val); \
        insert_last(head, val); \
        print(*head); \
    } while(0)

// Insert at position and print
#define INSERT_POS_AND_PRINT(head, val, pos, msg) \
    do { \
        printf("%s: %d at position %d\n", msg, val, pos); \
        insert_pos(head, val, pos); \
        print(*head); \
    } while(0)

// Delete first and print
#define DELETE_FIRST_AND_PRINT(head, msg) \
    do { \
        printf("%s\n", msg); \
        delete_first(head); \
        print(*head); \
    } while(0)

// Delete last and print
#define DELETE_LAST_AND_PRINT(head, msg) \
    do { \
        printf("%s\n", msg); \
        delete_end(head); \
        print(*head); \
    } while(0)

// Delete at position and print
#define DELETE_POS_AND_PRINT(head, pos, msg) \
    do { \
        printf("%s: position %d\n", msg, pos); \
        delete_pos(head, pos); \
        print(*head); \
    } while(0)

// Chain multiple operations
#define CHAIN_OPS(head, ...) \
    do { \
        __VA_ARGS__ \
    } while(0)

//FUNCTIONS
//
// Create new NODE function
// @arg1: @int DATA; data stored in node
//   and create a NULL 'next' point of the node, to allow growth.
// @return node
static struct Node *create_node(int data)
{
    struct Node *new_node = (struct Node*)malloc(sizeof(struct Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

// INSERT a new instance of data at the beginning of the linked list.
static void insert_first(struct Node **head, int data)
{
    struct Node *new_node = create_node(data);
    new_node->next = *head;
    *head = new_node;
}

// INSERT a new instance of data at the end of the linked list.
static void insert_last(struct Node **head, int data)
{
    struct Node *new_node = create_node(data);
    if (*head == NULL)
    {
        *head = new_node;
        return;
    }

    struct Node *temp = *head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new_node;
}

// INSERT a new instance at a specific position of the linked list.
// Position is 0-based indexing (0 = first, 1 = second, etc.)
static void insert_pos(struct Node **head, int data, int pos)
{
    if (pos < 0) {
        printf("Invalid position: position cannot be negative\n");
        return;
    }
    
    if (pos == 0)
    {
        insert_first(head, data);
        return;
    }

    struct Node *new_node = create_node(data);
    struct Node *temp = *head;
    
    // Traverse to the node just before the insertion point
    for (int i = 0; temp != NULL && i < pos - 1; i++)
    {
        temp = temp->next;
    }
    
    if (temp == NULL)
    {
        printf("Position out of bounds\n");
        free(new_node);
        return;
    }
    
    new_node->next = temp->next;
    temp->next = new_node;
}

// DELETE first instance of the linked list
static void delete_first(struct Node **head)
{
    if (*head == NULL)
    {
        printf("List is empty\n");
        return;
    }
    struct Node *temp = *head;
    *head = temp->next;
    free(temp);
}

// DELETE last instance of the linked list
static void delete_end(struct Node **head)
{
    if (*head == NULL)
    {
        printf("List is empty\n");
        return;
    }
    
    struct Node *temp = *head;
    
    // If only one node
    if (temp->next == NULL)
    {
        free(temp);
        *head = NULL;
        return;
    }
    
    // Traverse to second-to-last node
    while (temp->next->next != NULL)
    {
        temp = temp->next;
    }
    free(temp->next);
    temp->next = NULL;
}

// DELETE specific pos of the linked list (0-based indexing)
static void delete_pos(struct Node **head, int pos)
{
    if (pos < 0) {
        printf("Invalid position: position cannot be negative\n");
        return;
    }
    
    if (*head == NULL)
    {
        printf("List is empty\n");
        return;
    }
    
    struct Node *temp = *head;
    
    if (pos == 0) {
        delete_first(head);
        return;
    }
    
    // Traverse to the node just before the one to delete
    for (int i = 0; temp != NULL && i < pos - 1; i++)
    {
        temp = temp->next;
    }
    
    if (temp == NULL || temp->next == NULL)
    {
        printf("Position out of bounds\n");
        return;
    }
    
    struct Node *to_delete = temp->next;
    temp->next = to_delete->next;
    free(to_delete);
}

// PRINT the linked list
static void print(struct Node *head)
{
    struct Node *temp = head;
    while (temp != NULL)
    {
        printf("%d -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

// now run it all

int main() {
    struct Node *head = NULL;
    
    printf("LINKED LIST OPERATIONS\n\n");
    
    // Using macros for cleaner testing
    INSERT_FIRST_AND_PRINT(&head, 10, "Inserting at beginning");
    INSERT_LAST_AND_PRINT(&head, 20, "Inserting at end");
    INSERT_LAST_AND_PRINT(&head, 5, "Inserting at end");
    INSERT_LAST_AND_PRINT(&head, 30, "Inserting at end");
    INSERT_POS_AND_PRINT(&head, 15, 2, "Inserting at position");
    
    DELETE_FIRST_AND_PRINT(&head, "Deleting first node");
    DELETE_LAST_AND_PRINT(&head, "Deleting last node");
    DELETE_POS_AND_PRINT(&head, 1, "Deleting at position");
    
    printf("\nDEMONSTRATING MORE OPERATIONS\n\n");
    
    // Reset list
    head = NULL;
    
    // Chain operations together
    CHAIN_OPS(&head,
        INSERT_FIRST_AND_PRINT(&head, 100, "Inserting at beginning");
        INSERT_LAST_AND_PRINT(&head, 200, "Inserting at end");
        INSERT_LAST_AND_PRINT(&head, 300, "Inserting at end");
        INSERT_POS_AND_PRINT(&head, 150, 1, "Inserting at position");
        DELETE_FIRST_AND_PRINT(&head, "Deleting first node");
        DELETE_LAST_AND_PRINT(&head, "Deleting last node");
    );
    
    printf("\nPRINTING RESULTS, CHECK CODE FOR LOGIC\n\n");
    head = NULL;
    
    insert_first(&head, 10);
    PRINT_MSG(head, "After inserting 10 at beginning");
    
    insert_last(&head, 20);
    PRINT_MSG(head, "After inserting 20 at end");
    
    insert_last(&head, 5);
    PRINT_MSG(head, "After inserting 5 at end");
    
    insert_last(&head, 30);
    PRINT_MSG(head, "After inserting 30 at end");
    
    insert_pos(&head, 15, 2);
    PRINT_MSG(head, "After inserting 15 at position 2");
    
    delete_first(&head);
    PRINT_MSG(head, "After deleting first node");
    
    delete_end(&head);
    PRINT_MSG(head, "After deleting last node");
    
    delete_pos(&head, 1);
    PRINT_MSG(head, "After deleting at position 1");
    
    // Clean up
    while (head != NULL) {
        delete_first(&head);
    }
    
    return 0;
}
