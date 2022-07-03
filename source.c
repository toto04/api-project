#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** UTILS **/

// /**
//  * @brief takes the possible characters of the words and converts
//  *
//  * '-': 0
//  * '[0-9]': [1-10]
//  * '[A-B]': [11-36]
//  * '_': 37
//  * '[a-b]': [38-63]
//  *
//  * @param c the input char
//  * @return char - the index for any array cointaining data about each char
//  */
// char char_to_index(char c) {
//     c -= 45;  // '-' = 0
//     if (c) {
//         c -= 2;  // digits are from 1 to 10
//         if (c > 11) {
//             c -= 7;
//             if (c > 37) {
//                 c -= 4;
//                 if (c > 38)
//                     c--;
//             }
//         }
//     }
//     return c;
// }

typedef struct letter_node {
    char letter;
    struct letter_node* next;
    struct letter_node* inner;
} letter_node;

/**
 * @brief creates a new node with the correct letter, just a wrapper to set everything correctly
 * @param c the letter of the node
 * @return letter_node* pointer to the node
 */
letter_node* new_node(char c) {
    letter_node* node = (letter_node*)malloc(sizeof(letter_node));
    node->letter = c;
    node->inner = NULL;
    node->next = NULL;
    return node;
}

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

/** MAIN **/

bool playing = false;
unsigned int word_length;

char* the_word = NULL;
int guesses = 0;

letter_node* vocabulary;

void append_word(char* word) {
    if (vocabulary == NULL) {
        // initialize the vocab if it's empty
        vocabulary = new_node(word[0]);
    }

    if (vocabulary->letter > word[0]) {
        // shift the vocab if the first letter is too big
        letter_node* n = vocabulary;
        vocabulary = new_node(word[0]);
        vocabulary->next = n;
    }

    letter_node* node = vocabulary;
    for (int i = 0; i < word_length; i++) {
        if (i) {
            // once the correct node is found, proceed with inserting the next one
            // only after the first word
            if (node->inner == NULL) {
                node->inner = new_node(word[i]);
            }
            node = node->inner;
        }

        while (node->letter != word[i]) {
            // navigate through the letters until the correct one is either found or inserted
            if (node->next == NULL) {
                node->next = new_node(word[i]);
                node = node->next;
            } else if (node->next->letter > word[i]) {
                letter_node* nn = new_node(word[i]);
                nn->next = node->next;
                node->next = nn;
                node = nn;
            } else {
                node = node->next;
            }
        }
    }
}

void print_inner(letter_node* node, char* word, int depth) {
    while (node != NULL) {
        word[depth] = node->letter;  // set the letter in the print string

        // TODO: filter the word

        if (depth == word_length - 1) {
            // once the bottom of the word is reached, print it and return
            puts(word);
            return;
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
    char* word = malloc(word_length);
    print_inner(vocabulary, word, 0);
    free(word);
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
        // set the word
        the_word = malloc(sizeof(char) * (word_length + 1));
        strncpy(the_word, read_line(), word_length);
        // set number of guesses
        guesses = atoi(read_line());
        // start the game
        playing = true;
    } else if (strcmp(cmd, "+inserisci_inizio") == 0) {
        // go back to inserting
        playing = false;
    } else if (strcmp(cmd, "+inserisci_fine") == 0) {
        // resume the game
        playing = true;
    } else if (strcmp(cmd, "+stampa_filtrate") == 0) {
        print_words();
    }
}

int main(int argc, char* argv[]) {
    // firts thing: load the word length
    word_length = atoi(read_line());
    char* line;

    while ((line = read_line()) != NULL) {
        if (line[0] == '+') {
            // parse the command if the lines starts with +
            parse_command(line);
            continue;
        }

        if (playing) {
            // play the game
            bool exact = true;
            for (unsigned int i = 0; i < word_length; i++) {
                if (the_word[i] != line[i]) {
                    exact = false;
                    break;
                }
            }
            if (exact) {
                // the word is found
                puts("ok");
                playing = false;
                continue;
            }
            guesses--;
            if (guesses == 0) {
                // the word is not found
                puts("ko");
                playing = false;
                continue;
            }
        } else {
            // append the word to the list
            char* nw = malloc(sizeof(char) * (word_length + 1));
            strncpy(nw, line, word_length);
            append_word(nw);
        }

        free(line);
    }
}