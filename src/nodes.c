#include "globals.c"

/**
 * the vocabulary is constructed in a weird way, let's say we have two words:
 * - aaaaa
 * - aabbb
 *
 * each word is its own array of nodes, beginning from the first different one, as follows:
 *
 * vocab:
 * [0] letter: "a"
 *     unviable: false
 *     next: NULL
 * [1] letter: "a"
 *     unviable: false
 *     next: NULL
 * [2] letter: "a"
 *     unviable: false
 *     next             -> next node:
 * [3] letter: "a"         | [0] letter: "b"
 *     unviable: false     |     unvaible: false
 *     next: NULL          |     next: NULL
 * [4] letter: "a"         | [1] letter: "b"
 *     unviable: false     |     unvaible: false
 *     next: NULL          |     next: NULL
 *
 * this way n nodes can be shared with word with the same first n letters
 * and the unviable flag can be set for just one node and automatically exclude all the following
 * words sharing said node without checking each of the letters
 */
typedef struct letter_node {
    char letter;
    char unviable;
    struct letter_node* next;
} __attribute__((packed)) letter_node;

letter_node* vocabulary;

/**
 * @brief creates a new node with the correct letter, just a wrapper to set everything correctly
 * @param c the letter of the node
 * @return letter_node* pointer to the node
 */
letter_node* new_node(char* c, unsigned int depth) {
    unsigned int len = word_length - depth;
    letter_node* node = (letter_node*)malloc(sizeof(letter_node) * len);
    for (int i = 0; i < word_length - depth; i++) {
        letter_node* n = &(node[i]);
        n->letter = c[i];
        n->unviable = false;
        n->next = NULL;
    }
    return node;
}

/**
 * @brief creates a new node with all the data from another one
 *
 * @param source the source form which to copy the node, can be in any position of the node array
 * @param depth the beginning position of the node array in the word
 * @return letter_node* pointer to the newly allocated node
 */
letter_node* copy_node(letter_node* source, unsigned int depth) {
    unsigned int len = word_length - depth;
    letter_node* node = (letter_node*)malloc(sizeof(letter_node) * len);
    memcpy(node, source, sizeof(letter_node) * len);
    return node;
}

/**
 * @brief recursevly navigates each node reconstructing the word, prints it if it's viable
 *
 * @param node the node array to iterate on
 * @param word the partially reconstructed node, each letter will be appendend to this string
 * @param depth the starting position in the word of the node array
 */
void print_inner(letter_node* node, char* word, int depth) {
    int i;
    int term = word_length - depth;
    for (i = 0; i < term; i++) {
        letter_node n = node[i];
        if (n.unviable) break;  // stop navigating if the word is not viable
        word[depth + i] = n.letter;
    }
    if (i == term) {
        i--;         // this will be one more than the last index for a letter
        puts(word);  // prints the word
    }
    for (int j = i; j >= 0; j--) {
        // go back from the last node (not deeper than an unviable one) and print from there
        // done this way to ensure correct order of printing
        letter_node n = node[j];
        if (n.next != NULL) {
            print_inner(n.next, word, j + depth);
        }
    }
}