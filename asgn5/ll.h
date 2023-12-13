
#include <stdbool.h>
#include <stddef.h>

typedef struct Node {
    char* data;
    int ref; 
    struct Node* next;
} Node;

Node* createNode(char* data);

Node* get(Node* head, char *data);

Node* get_index(Node* head, int index);

void deleteList(Node** head);

void removeHead(Node** head);

void addEnd(Node** head, char* data);

void moveEnd(Node **head, char *data);

int overwrite(Node* head, int index, char* data);

int list_is_full(Node* head, int n);

void printList(Node* head);
