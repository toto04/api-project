#ifndef UTILS
#define UTILS

#include <stdlib.h>
#include <stdbool.h>

#include "globals.c"

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

/**
 * @brief initializes the letter lookup table using the letter_to_index function
 * using such table is way faster than calling the function each time
 */
void initialize_letter_lookup() {
    for (int i = 0; i < 256; i++) {
        letter_lookup[i] = letter_to_index((char)i);
    }
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

#endif