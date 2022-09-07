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

#endif