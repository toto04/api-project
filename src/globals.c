#ifndef GLOBALS
#define GLOBALS

#include <stdlib.h>
#include <stdbool.h>

bool playing = false;
bool playing_prev = false;
char* the_word = NULL;
unsigned int word_length;
unsigned int char_count[64];
int guesses = 0;
int viable_count = 0;

// insert the char as the index and out comes the normalized index!
unsigned short letter_lookup[256] = {0};

#endif