typedef struct letter_node {
    char letter;
    char unviable;
    struct letter_node* next;
    struct letter_node* inner;
} __attribute__((packed)) letter_node;

letter_node* vocabulary;

/**
 * @brief creates a new node with the correct letter, just a wrapper to set everything correctly
 * @param c the letter of the node
 * @return letter_node* pointer to the node
 */
letter_node* new_node(char c, bool unviability) {
    letter_node* node = (letter_node*)malloc(sizeof(letter_node));
    node->letter = c;
    node->unviable = unviability;
    node->inner = NULL;
    node->next = NULL;
    return node;
}

void print_inner(letter_node* node, char* word, int depth) {
    while (node != NULL) {
        // if (word[0] == 'A' && word[1] == 'A' && word[2] == 'm' && depth >= 2) {
        //     printf("");
        // }

        if (node->unviable) {
            node = node->next;
            continue;
        }
        word[depth] = node->letter;  // set the letter in the print string

        // if (node->unviable) {
        //     word[depth] = '%';
        // }

        if (depth == word_length - 1) {
            // once the bottom of the word is reached, print it and return
            word[word_length] = '\0';
            puts(word);
            node = node->next;
            continue;
        }

        // first print all the way to the bottom
        print_inner(node->inner, word, depth + 1);
        // then print the next word, from the innermost difference (too preserve order)
        node = node->next;
    }
}