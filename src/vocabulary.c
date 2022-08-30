#include "nodes.c"

/**
 * @brief appends a word to the vocabolary tree, managing the node position for each letter
 *
 * @param word the word to be appended
 */
void append_word(char* word);

/**
 * @brief checks if a letter node is viable for a given position and previous letter counts
 *
 * @param letter the char to be checked for viability
 * @param letter_count array of length 64 containing the number of times each letter has been found
 * @param depth the depth of the letter, 0 for the first letter, 1 for the second, etc.
 * @return true if the letter is unviable, false otherwise
 */
bool check_viability(char letter, unsigned int* letter_count, int depth);

/**
 * @brief reursevly sets the unviable flag to false for each inner node of a letter node
 *
 * @param node the node to be cleared
 */
void clear_viability(letter_node* node);

void append_word(char* word) {
    if (vocabulary == NULL) {
        // initialize the vocab if it's empty
        vocabulary = new_node(word[0], false);
    }

    if (vocabulary->letter > word[0]) {
        // shift the vocab if the first letter is too big
        letter_node* n = vocabulary;
        vocabulary = new_node(word[0], false);
        vocabulary->next = n;
    }

    letter_node* node = vocabulary;
    letter_node* parent = NULL;
    unsigned int letter_count[64] = {0};
    for (int i = 0; i < word_length; i++) {
        letter_count[letter_to_index(word[i])]++;
        if (i) {
            // once the correct node is found, proceed with inserting the next one
            // only after the first word
            if (node->inner == NULL) {
                node->inner = new_node(word[i], check_viability(word[i], letter_count, i));
            }
            parent = node;
            node = node->inner;
        }

        while (node->letter != word[i]) {
            // navigate through the letters until the correct one is either found or inserted

            if (node->letter > word[i]) {
                letter_node* nn = new_node(word[i], check_viability(word[i], letter_count, i));
                nn->next = node;
                node = nn;
                parent->inner = node;
            } else if (node->next == NULL) {
                node->next = new_node(word[i], check_viability(word[i], letter_count, i));
                node = node->next;
            } else if (node->next->letter > word[i]) {
                letter_node* nn = new_node(word[i], check_viability(word[i], letter_count, i));
                nn->next = node->next;
                node->next = nn;
                node = nn;
            } else {
                node = node->next;
            }
        }
    }
}

bool check_viability(char letter, unsigned int* letter_count, int depth) {
    if (!constraints.found_chars) return false;

    unsigned short idx = letter_to_index(letter);

    // check absent constraint
    if (constraints.absent[idx]) {
        return true;
    }

    // check found constraint
    if (constraints.found_chars[depth] && constraints.found_chars[depth] != letter) {
        return true;
    }

    // check impossible constraint
    impossible_node* imposs = constraints.impossible_chars[depth].head;
    while (imposs != NULL) {
        if (imposs->letter == letter)
            return true;
        imposs = imposs->next;
    }

    short d = 4 - depth;
    for (int i = 0; i < 64; i++) {
        // check the minimum constraint
        // if there cannot possibly be enough letters to satisfy the minimum constraint, then the letter is unviable
        if (letter_count[i] + d < constraints.minimum[i])
            return true;
        // once the letter count passes the exact constraint, it is unviable
        if (letter_count[i] > constraints.exact[i])
            return true;
    }

    return false;
}

void clear_viability(letter_node* node) {
    if (node == NULL)
        return;
    node->unviable = false;
    clear_viability(node->inner);
    clear_viability(node->next);
}

void update_viability_inner(letter_node* node, unsigned int* letter_count, int depth) {
    if (node == NULL) {
        viable_count++;
        return;
    }
    while (node != NULL) {
        unsigned short idx = letter_to_index(node->letter);
        letter_count[idx]++;
        node->unviable = check_viability(node->letter, letter_count, depth);
        if (!node->unviable) update_viability_inner(node->inner, letter_count, depth + 1);
        letter_count[idx]--;
        node = node->next;
    }
}

void update_viability() {
    viable_count = 0;
    letter_node* node = vocabulary;
    unsigned int letter_count[64] = {0};
    update_viability_inner(node, letter_count, 0);
}