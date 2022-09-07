#include "constraints.c"
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
void clear_viability(letter_node* node, unsigned depth);

void append_word(char* word) {
    if (vocabulary == NULL) {
        vocabulary = new_node(word, 0);
        return;
    }

    letter_node* current_node = vocabulary;
    unsigned current_depth = 0;
    unsigned int letter_count[64] = {0};
    // bool already_unviable = false;
    for (int i = 0; i < word_length;) {
        letter_node* n = &(current_node[i - current_depth]);
        if (word[i] > n->letter) {
            if (n->next == NULL) {
                n->next = new_node(&(word[i]), i);
                for (int j = i; j < word_length; j++) {
                    letter_node* nn = &(n->next[j - i]);
                    letter_count[letter_to_index(word[j])]++;
                    bool unv = check_viability(word[j], letter_count, j);
                    nn->unviable = unv;
                    if (unv) break;
                }
                return;
            }
            current_node = n->next;
            current_depth = i;
            continue;
        }
        if (word[i] < n->letter) {
            letter_node* nn = copy_node(n, i);
            for (int j = 0; j < word_length - i; j++) {
                int idx = j + i;
                letter_node* k = &(current_node[idx - current_depth]);
                letter_count[letter_to_index(word[idx])]++;
                k->letter = word[idx];
                k->unviable = check_viability(word[idx], letter_count, idx);
                k->next = NULL;
            }
            n->next = nn;
            return;
        }
        // already_unviable = already_unviable && n->unviable;
        letter_count[letter_to_index(word[i])]++;
        i++;
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

    short d = word_length - depth;
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

void clear_viability(letter_node* node, unsigned depth) {
    for (int i = 0; i < word_length - depth; i++) {
        letter_node* n = &(node[i]);
        n->unviable = false;
        if (n->next) {
            clear_viability(n->next, depth + i);
        }
    }
}

void update_viability_inner(letter_node* node, unsigned int* letter_count, int depth) {
    int i;
    for (i = 0; i < word_length - depth; i++) {
        letter_node* n = &(node[i]);
        if (n->next) update_viability_inner(n->next, letter_count, depth + i);
        unsigned short idx = letter_to_index(n->letter);
        letter_count[idx]++;
        bool unv = check_viability(n->letter, letter_count, depth + i);
        n->unviable = unv;
        if (unv)
            break;
    }
    if (i == word_length - depth)
        viable_count++;
    else
        i++;
    // reset the letter_count table before returning
    for (int j = 0; j < i; j++) {
        letter_node n = node[j];
        unsigned short idx = letter_to_index(n.letter);
        letter_count[idx]--;
    }
}

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