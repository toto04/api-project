#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool playing = false;
char* the_word = NULL;
unsigned int word_length;
unsigned int char_count[64];
int guesses = 0;

/** UTILS **/

/**
 * @brief takes the possible characters of the words and converts
 *
 * '-': 0
 * '[0-9]': [1-10]
 * '[A-B]': [11-36]
 * '_': 37
 * '[a-b]': [38-63]
 *
 * @param c the input char
 * @return char - the index for any array cointaining data about each char
 */
unsigned short letter_to_index(char c) {
    c -= 45;  // '-' = 0
    if (c) {
        c -= 2;  // digits are from 1 to 10
        if (c > 11) {
            c -= 7;
            if (c > 37) {
                c -= 4;
                if (c > 38)
                    c--;
            }
        }
    }
    return c;
}

// char index_to_letter(unsigned short i) {
//     if (i == 0)
//         return '-';
//     if (i < 11)
//         return i + '0' - 1;
//     if (i < 37) return i + 'A' - 11;
//     if (i == 37)
//         return '_';
//     return i + 'a' - 38;
// }

// unsigned short* create_letter_table() {
//     static unsigned short table[64];
//     for (int i = 0; i < 64; i++)
//         table[i] = 0;
//     return table;
// }

typedef struct letter_node {
    char letter;
    bool unviable;
    struct letter_node* next;
    struct letter_node* inner;
} __attribute__((packed)) letter_node;

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

letter_node* vocabulary;

/**
 * @brief reads one incoming line
 * @return either a string or null
 */
char* read_line() {
    char* line = NULL;
    size_t l = 0;

    if (getline(&line, &l, stdin) != -1)
        return line;

    return NULL;
}

/** CONSTRAINTS **/

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
        constraints.found_chars[i] = '%';
        constraints.impossible_chars[i].head = NULL;
        constraints.impossible_chars[i].tail = NULL;
    }

    // reset everything
    for (int i = 0; i < 64; i++) {
        constraints.absent[i] = false;
        constraints.minimum[i] = 0;
        constraints.exact[i] = 0;
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

/** MAIN **/

/**
 * @brief checks if a letter node is viable for a given position and previous letter counts
 *
 * @param letter the char to be checked for viability
 * @param letter_count array of length 64 containing the number of times each letter has been found
 * @param depth the depth of the letter, 0 for the first letter, 1 for the second, etc.
 * @return true if the letter is unviable, false otherwise
 */
bool check_viability(char letter, unsigned int* letter_count, int depth) {
    if (!constraints.found_chars) return false;

    unsigned short idx = letter_to_index(letter);

    // check absent constraint
    if (constraints.absent[idx]) {
        return true;
    }

    // check found constraint
    if (constraints.found_chars[depth] != '%' && constraints.found_chars[depth] != letter) {
        return true;
    }

    // check impossible constraint
    impossible_node* imposs = constraints.impossible_chars[depth].head;
    while (imposs != NULL) {
        if (imposs->letter == letter)
            return true;
        imposs = imposs->next;
    }

    for (int i = 0; i < 64; i++) {
        // check the minimum constraint
        // if there cannot possibly be enough letters to satisfy the minimum constraint, then the letter is unviable
        if (letter_count[i] + 4 - depth < constraints.minimum[i])
            return true;
        // once the letter count passes the exact constraint, it is unviable
        if (constraints.exact[i] > 0 && letter_count[i] > constraints.exact[i])
            return true;
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
                if (parent) {
                    parent->inner = node;
                } else {
                    puts("something went wrong");
                }
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

void clear_viability(letter_node* node) {
    if (node == NULL)
        return;
    node->unviable = false;
    clear_viability(node->inner);
    clear_viability(node->next);
}

void update_viability_inner(letter_node* node, unsigned int* letter_count, int depth) {
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
    letter_node* node = vocabulary;
    unsigned int letter_count[64] = {0};
    update_viability_inner(node, letter_count, 0);
}

bool word_exists(char* word) {
    letter_node* node = vocabulary;
    for (int i = 0; i < word_length; i++) {
        char letter = word[i];
        while (node->letter != letter) {
            if (node->next == NULL)
                return false;
            node = node->next;
        }
        node = node->inner;
    }
    return true;
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

/**
 * @brief prints the filtered word list
 */
void print_words() {
    char* word = malloc(sizeof(char) * word_length + 1);
    print_inner(vocabulary, word, 0);
    free(word);
}

int count_viable_words(letter_node* node) {
    unsigned int c = 0;
    while (node != NULL) {
        if (!node->unviable) {
            if (node->inner == NULL) {
                c++;
            } else {
                c += count_viable_words(node->inner);
            }
        }
        node = node->next;
    }
    return c;
}

/**
 * @brief parse a command from an stdin line, and executes it
 * @param command the line from stdin
 */
void parse_command(char* cmd) {
    // OPT: maybe its too much? just return the line in case
    strtok(cmd, "\n");  // reallocate the string to match the correct length

    // printf("command: %s, %d\n", command, );
    if (strcmp(cmd, "+nuova_partita") == 0) {
        // initialize the constraints
        initialize_contraints();
        // clear the viability
        clear_viability(vocabulary);
        // set the word
        if (the_word) free(the_word);
        the_word = malloc(sizeof(char) * (word_length + 1));
        char* l = read_line();
        strncpy(the_word, l, word_length);
        free(l);
        // count each letter
        for (int i = 0; i < 64; i++) char_count[i] = 0;
        for (int i = 0; i < word_length; i++) {
            char c = the_word[i];
            unsigned short index = letter_to_index(c);
            char_count[index]++;
        }

        // set number of guesses
        l = read_line();
        guesses = atoi(l);
        free(l);
        // start the game
        playing = true;
    } else if (strcmp(cmd, "+inserisci_inizio") == 0) {
        // go back to inserting
        playing = false;
    } else if (strcmp(cmd, "+inserisci_fine") == 0) {
        // resume the game
        playing = true;
        // recalculate the viability of the letters
    } else if (strcmp(cmd, "+stampa_filtrate") == 0) {
        print_words();
    }
}

int main(int argc, char* argv[]) {
    // firts thing: load the word length
    char* line = read_line();
    word_length = atoi(line);
    free(line);

    while ((line = read_line()) != NULL) {
        if (line[0] == '+') {
            // parse the command if the lines starts with +
            parse_command(line);
        } else if (playing) {
            // play the game
            bool exact = true;
            if (word_exists(line)) {
                unsigned int printed_char_count[64] = {0};
                char* response = malloc(sizeof(char) * word_length + 1);
                response[word_length] = '\0';

                for (unsigned int i = 0; i < word_length; i++) {
                    char c = line[i];
                    if (the_word[i] == c) {
                        unsigned short index = letter_to_index(c);
                        printed_char_count[index]++;  // count the letters
                        // set the constraint for the found letter
                        constraints.found_chars[i] = c;
                        response[i] = '+';
                    }
                }

                for (unsigned int i = 0; i < word_length; i++) {
                    char c = line[i];
                    if (the_word[i] == c) continue;  // already handled

                    unsigned short idx = letter_to_index(c);

                    unsigned int lettercount = char_count[idx];

                    if (lettercount == 0)
                        constraints.absent[idx] = true;
                    else
                        append_impossible_char(i, c);

                    if (printed_char_count[idx] < lettercount) {
                        printed_char_count[idx]++;  // increase the printed letter count

                        // set the minimum constraint
                        if (printed_char_count[idx] > constraints.minimum[idx])
                            constraints.minimum[idx] = printed_char_count[idx];

                        response[i] = '|';  // print | for a char in the wrong place
                    } else {
                        // set the exact constraint
                        constraints.minimum[idx] = lettercount;
                        constraints.exact[idx] = lettercount;
                        response[i] = '/';  // print / for an absent char
                    }

                    exact = false;  // set the exact flag to false
                }

                if (exact) {
                    // the word is found
                    puts("ok");
                    playing = false;
                } else {
                    update_viability();  // update the viability of the letters

                    // prints the response
                    puts(response);
                    // prints how many words are remaining
                    int viable_words = count_viable_words(vocabulary);
                    printf("%d\n", viable_words);
                    guesses--;

                    if (guesses == 0) {
                        // no more guesses, game lost
                        puts("ko");
                        playing = false;
                    }
                }

                free(response);
            } else {
                puts("not_exists");
            }
        } else {
            // append the word to the list
            append_word(line);
        }

        free(line);
    }
}