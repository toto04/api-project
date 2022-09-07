#include "globals.c"

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

letter_node* copy_node(letter_node* source, unsigned int depth) {
    unsigned int len = word_length - depth;
    letter_node* node = (letter_node*)malloc(sizeof(letter_node) * len);
    memcpy(node, source, sizeof(letter_node) * len);
    return node;
}

void print_inner(letter_node* node, char* word, int depth) {
    int i;
    int term = word_length - depth;
    for (i = 0; i < term; i++) {
        letter_node n = node[i];
        if (n.unviable) break;
        word[depth + i] = n.letter;
    }
    if (i == term) {
        i--;
        puts(word);
    }
    for (int j = i; j >= 0; j--) {
        letter_node n = node[j];
        if (n.next != NULL) {
            print_inner(n.next, word, j + depth);
        }
    }
}