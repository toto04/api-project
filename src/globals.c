#ifndef GLOBALS
#define GLOBALS

#include <stdlib.h>
#include <stdbool.h>

bool playing = false;         // the current state of the game
bool playing_prev = false;    // used to reset the state after +inserisci_fine, could be called while
                              // playing is still false
char* the_word = NULL;        // the correct word for the game
unsigned int word_length;     // length for each word
unsigned int char_count[64];  // letter count table for the correct word
int guesses = 0;              // number of guesses available for the game
int viable_count = 0;         // global count of how many words are still viable

// insert the char as the index and out comes the normalized index!
unsigned short letter_lookup[256] = {0};

#endif