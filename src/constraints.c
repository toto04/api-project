#ifndef CONSTRAINTS
#define CONSTRAINTS

#include <stdlib.h>
#include <stdbool.h>
#include "globals.c"

typedef struct impossible_node {
    char letter;
    struct impossible_node* next;
} impossible_node;

typedef struct impossible_list {
    impossible_node* head;
    impossible_node* tail;
} impossible_list;

typedef struct constraints_str {
    bool absent[64];
    char* found_chars;
    impossible_list* impossible_chars;
    unsigned int minimum[64];
    unsigned int exact[64];
} constraints_str;

constraints_str constraints;

void initialize_contraints() {
    // free the impossible_chars lists
    if (constraints.impossible_chars) {
        for (int i = 0; i < word_length; i++) {
            impossible_node* node = constraints.impossible_chars[i].head;
            while (node) {
                impossible_node* next = node->next;
                free(node);
                node = next;
            }
        }
        free(constraints.impossible_chars);
    }
    constraints.impossible_chars = malloc(sizeof(impossible_list) * word_length);

    if (constraints.found_chars)
        free(constraints.found_chars);
    constraints.found_chars = (char*)malloc(word_length * sizeof(char));

    for (int i = 0; i < word_length; i++) {
        constraints.found_chars[i] = '\0';
        constraints.impossible_chars[i].head = NULL;
        constraints.impossible_chars[i].tail = NULL;
    }

    // reset everything
    for (int i = 0; i < 64; i++) {
        constraints.absent[i] = false;
        constraints.minimum[i] = 0;
        constraints.exact[i] = __UINT32_MAX__;
    }
}

void append_impossible_char(unsigned short pos, char c) {
    impossible_node* node = (impossible_node*)malloc(sizeof(impossible_node));
    node->letter = c;
    node->next = NULL;

    if (constraints.impossible_chars[pos].tail == NULL) {
        constraints.impossible_chars[pos].head = node;
        constraints.impossible_chars[pos].tail = node;
    } else {
        constraints.impossible_chars[pos].tail->next = node;
        constraints.impossible_chars[pos].tail = node;
    }
}

#endif