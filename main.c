#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/globals.c"
#include "src/utils.c"
#include "src/constraints.c"
#include "src/vocabulary.c"

/** MAIN **/

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
        clear_viability(vocabulary, 0);
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
        playing_prev = playing;
        playing = false;
    } else if (strcmp(cmd, "+inserisci_fine") == 0) {
        // resume the game
        playing = playing_prev;
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

    initialize_contraints();  // just to be sure

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
                    printf("%d\n", viable_count);
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