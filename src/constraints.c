#ifndef CONSTRAINTS
#define CONSTRAINTS

#include <stdlib.h>
#include <stdbool.h>
#include "globals.c"

/**
 * @brief node with the index of a char, only these indices will be checked for the minimum constraint
 */
typedef struct minimum_check_node {
    unsigned short index;
    struct minimum_check_node* next;
} minimum_check_node;

/**
 * @brief node of a list with a char that cannot appear in a position
 */
typedef struct impossible_node {
    char letter;
    struct impossible_node* next;
} impossible_node;

/**
 * @brief list with head and tail of impossible chars
 */
typedef struct impossible_list {
    impossible_node* head;
    impossible_node* tail;
} impossible_list;

/**
 * @brief struct containing all the constraints
 */
typedef struct constraints_str {
    bool absent[64];                         // array of bools, true if the letter is absent
    char* found_chars;                       // string with the chars with known position in the word
    minimum_check_node* minimum_check_list;  // list of indices of chars that have a minimum constraint
    impossible_list* impossible_chars;       //[word_length];  // array of lists of chars that cannot appear in a position
    unsigned int minimum[64];                // array of minimum letter counts, 0 if no minimum
    unsigned int exact[64];                  // array of exact letter counts, __UINT32_MAX__ if no exact
} constraints_str;

constraints_str constraints;

/**
 * @brief resets all the contraints to the default value
 */
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

    // free the minimum_check_list
    if (constraints.minimum_check_list) {
        minimum_check_node* node = constraints.minimum_check_list;
        while (node) {
            minimum_check_node* next = node->next;
            free(node);
            node = next;
        }
    }
    constraints.minimum_check_list = NULL;

    // free the found_chars string
    if (constraints.found_chars)
        free(constraints.found_chars);
    constraints.found_chars = (char*)malloc(word_length * sizeof(char));

    for (int i = 0; i < word_length; i++) {
        // all chars in the found chars string are set to 0
        constraints.found_chars[i] = '\0';
        // all entries in the impossible chars array must be set to empty
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

/**
 * @brief appends a char to the minimum_check_list
 *
 * @param index the index of the char that must be checked
 */
void append_minimum_check(unsigned short index) {
    minimum_check_node* node = malloc(sizeof(minimum_check_node));
    node->index = index;
    node->next = NULL;

    if (constraints.minimum_check_list != NULL) {
        minimum_check_node* last = constraints.minimum_check_list;
        while (last->next != NULL)
            last = last->next;
        last->next = node;
    } else {
        constraints.minimum_check_list = node;
    }
}

/**
 * @brief appends a char to the impossible_chars list for a given position
 *
 * @param pos the position in which the char cannot appear
 * @param c the char that cannot appear
 */
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