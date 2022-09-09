#include "constraints.c"
#include "nodes.c"

/**
 * @brief checks if a letter node is viable for a given position and previous letter counts
 *
 * @param letter the char to be checked for viability
 * @param letter_count array of length 64 containing the number of times each letter has been found
 * @param depth the depth of the letter, 0 for the first letter, 1 for the second, etc.
 * @return true if the letter is unviable, false otherwise
 */
bool check_viability(char letter, unsigned int* letter_count, int depth) {
    unsigned short idx = letter_lookup[(unsigned char)letter];

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

    // each letter has to be checked couse the depth changes between checks
    short d = word_length - depth - 1;
    for (int i = 0; i < 64; i++) {
        // the minimum constraint has to be set, otherwise its useless to check anything
        if (constraints.minimum[i]) {
            // check the minimum constraint
            // if there cannot possibly be enough letters to satisfy the minimum constraint, then the letter is unviable
            if (letter_count[i] + d < constraints.minimum[i])
                return true;
            // once the letter count passes the exact constraint, it is unviable
            if (letter_count[i] > constraints.exact[i])
                return true;
        }
    }

    return false;
}

/**
 * @brief appends a word to the vocabolary tree, managing the node position for each letter
 *
 * @param word the word to be appended
 */
void append_word(char* word) {
    if (vocabulary == NULL) {
        vocabulary = new_node(word, 0);
        return;
    }

    letter_node* current_node = vocabulary;  // begin at the start of the vocabulary
    unsigned current_depth = 0;              // the starting position for the current node, 0 for the vocabulary

    unsigned int letter_count[64] = {0};  // table with the count of each letter for check_viability
    bool already_unviable = false;        // used to avoid too many useless checks
    for (int i = 0; i < word_length;) {
        // navigate the whole length of the word
        letter_node* n = &(current_node[i - current_depth]);

        if (word[i] > n->letter) {
            // go to the next node since the letter is too small
            if (n->next == NULL) {
                // if theres no next node, create it
                unsigned int len = word_length - i;
                // allocating the new node, ad hoc bc the viability needs to be set the correct way
                n->next = (letter_node*)malloc(sizeof(letter_node) * len);
                for (int j = i; j < word_length; j++) {
                    letter_node* nn = &(n->next[j - i]);
                    letter_count[letter_lookup[(unsigned char)word[j]]]++;

                    nn->letter = word[j];
                    nn->next = NULL;

                    // only set the viability if it's not already unviable, if it is it wont be checked anyway
                    nn->unviable = !already_unviable && check_viability(word[j], letter_count, j);
                    already_unviable = already_unviable || nn->unviable;
                }
                return;
            }

            // keep skipping until the letter is found
            current_node = n->next;
            current_depth = i;
            continue;
        }

        if (word[i] < n->letter) {
            // the letter should be inserted before the current node
            // next node will just be a copy of the current one
            // and the current will be overridden with the new word
            letter_node* nn = copy_node(n, i);
            for (int j = 0; j < word_length - i; j++) {
                // override the current node
                int idx = j + i;
                letter_node* k = &(current_node[idx - current_depth]);
                letter_count[letter_lookup[(unsigned char)word[idx]]]++;  // spaghett
                k->letter = word[idx];

                // the unviability is the biggest thing damn
                if (already_unviable) {
                    k->unviable = false;  // will never be checked anyway
                } else {
                    // check it again
                    bool unv = check_viability(word[idx], letter_count, idx);
                    k->unviable = unv;
                    already_unviable = already_unviable || unv;
                }
                k->next = NULL;
            }
            n->next = nn;
            return;
        }

        // going deeper in the word if the letter is right
        already_unviable = already_unviable || n->unviable;
        letter_count[letter_lookup[(unsigned char)word[i]]]++;
        i++;
    }
}

/**
 * @brief reursevly sets the unviable flag to false for each node, called when a new game begins
 *
 * @param node the node to be cleared
 */
void clear_viability(letter_node* node, unsigned depth) {
    for (int i = 0; i < word_length - depth; i++) {
        letter_node* n = &(node[i]);
        n->unviable = false;
        if (n->next) {
            clear_viability(n->next, depth + i);
        }
    }
}

/**
 * @brief checks the viability again for each node when a new constraint is set
 *
 * @param node the node array that needs to be updated
 * @param letter_count  the letter count table, updated for each letter and passed on recursively
 * @param depth the beginning position for the node array
 */
void update_viability_inner(letter_node* node, unsigned int* letter_count, int depth) {
    int i;
    for (i = 0; i < word_length - depth; i++) {
        letter_node* n = &(node[i]);
        if (n->next) update_viability_inner(n->next, letter_count, depth + i);
        unsigned short idx = letter_lookup[(unsigned char)n->letter];
        letter_count[idx]++;
        if (n->unviable) break;  // no need to check again
        if (check_viability(n->letter, letter_count, depth + i)) {
            n->unviable = true;
            break;
        }
    }
    if (i == word_length - depth)
        viable_count++;
    else
        i++;  // broke before last increment
    // reset the letter_count table before returning
    for (int j = 0; j < i; j++)
        letter_count[letter_lookup[(unsigned char)node[j].letter]]--;
}

/**
 * @brief updates the viabilities for the whole vocabulary
 */
void update_viability() {
    viable_count = 0;
    unsigned int letter_count[64] = {0};
    update_viability_inner(vocabulary, letter_count, 0);
}

/**
 * @brief prints the filtered word list
 */
void print_words() {
    char* word = malloc(sizeof(char) * word_length + 1);
    word[word_length] = '\0';
    print_inner(vocabulary, word, 0);
    free(word);
}

/**
 * @brief checks if a word is valid reconstructing it by navigating the vocabulary
 *
 * @param word the word to be checked
 * @return true if the word is part of the vocabulary
 * @return false if the word is not present
 */
bool word_exists(char* word) {
    letter_node* current_node = vocabulary;
    unsigned current_depth = 0;
    for (int i = 0; i < word_length; i++) {
        char letter = word[i];
        letter_node* node = &(current_node[i - current_depth]);
        while (node->letter != letter) {
            if (node->next == NULL)
                return false;
            current_node = node->next;
            current_depth = i;
            node = current_node;
        }
    }
    return true;
}