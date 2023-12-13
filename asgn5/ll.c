#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ll.h"

Node *createNode(char *data) {
    struct Node *newNode = (struct Node *) malloc(sizeof(struct Node));
    if (!newNode) {
        printf("Memory error\n");
        return NULL;
    }
    newNode->data = data;
    newNode->next = NULL;
    newNode->ref = 0;
    return newNode;
}

Node *get(Node *head, char *data) {
    // get the node
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->data, data) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Node *get_index(Node *head, int index) {
    // get the node at the index
    Node *current = head;
    int count = 0;
    while (current != NULL) {
        if (count == index)
            return (current);
        count++;
        current = current->next;
    }
    return NULL;
}

void deleteList(Node **head) {
    Node *currentNode = *head;
    Node *nextNode;

    while (currentNode != NULL) {
        nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }

    *head = NULL;
}

void removeHead(Node **head) {
    // remove the head node
    Node *temp = *head;
    if (temp == NULL)
        return;
    *head = temp->next;
    // avoid memory leak for original spot
    free(temp);
}

void addEnd(Node **head, char *data) {
    Node *newNode = createNode(data);
    // if head is null then the new node become the head
    if (*head == NULL) {
        *head = newNode;
    } else {
        //else add the node to the end
        Node *last = *head;
        while (last->next != NULL) {
            last = last->next;
        }

        last->next = newNode;
    }
}

void moveEnd(Node **head, char *data) {
    Node *temp = *head, *prev;

    // If head node itself holds the data to be moved to end
    if (temp != NULL && strcmp(temp->data, data) == 0) {
        *head = temp->next;
        temp->next = NULL;
        addEnd(head, temp->data);
        return;
    }

    // Find the node that holds the data to be moved to end
    while (temp != NULL && strcmp(temp->data, data) != 0) {
        prev = temp;
        temp = temp->next;
    }

    // If data was not present in linked list
    if (temp == NULL) {
        printf("Data not found in the linked list.\n");
        return;
    }

    // Remove the node from its current position
    prev->next = temp->next;

    // Move the node to the end of the list
    temp->next = NULL;
    addEnd(head, temp->data);
}

int overwrite(Node *head, int index, char *data) {
    Node *current = head;
    int count = 0;
    while (current != NULL) {
        if (count == index) {
            current->data = data;
            return 1;
        }
        count++;
        current = current->next;
    }
    return 0;
}

int list_is_full(Node *head, int n) {
    Node *current = head;
    int count = 0;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    if (count >= n) {
        return 1;
    }
    return 0;
}

// debug
void printList(Node *head) {
    Node *current = head;
    fprintf(stdout, "This is the list right now: \n");
    if (head == NULL) {
        fprintf(stdout, "NULL\n");
        return;
    }
    while (current != NULL) {
        fprintf(stdout, "%s ", (current->data));
        fprintf(stdout, " ");
        current = current->next;
    }
    fprintf(stdout, "\n");
}
