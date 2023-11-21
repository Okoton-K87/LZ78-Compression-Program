#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "word.h"
#include "code.h"

// typedef struct Word {
//     uint8_t *syms;
//     uint32_t len;
// } Word;

// Constructor for a word where sysms is the array of symbols a Word represents.
/*
 * Creates a new Word with symbols syms and length len
 * Allocates new array and copies the symbols over
 */
Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = (Word *) malloc(sizeof(Word));
    // This function returns a Word * if successful or NULL otherwise.
    if (w) {
        // The length of the array of symbols is given by len.
        w->syms = (uint8_t *) calloc(len + 1, sizeof(uint8_t));
        // copy chars in syms over
        for (uint32_t i = 0; i < len; i++) {
            w->syms[i] = syms[i];
        }
        w->len = len;
        return w;
    } else {
        return NULL;
    }
}

// Constructs a new Word from the specified Word, w, appended with a symbol, sym.
/*
 * Creates a new word by appending symbol sym to word w
 * Updates the length of the new word and copies symbols over
 * Returns a pointer to the newly allocated word
 */
Word *word_append_sym(Word *w, uint8_t sym) {
    uint32_t new_len = w->len + 1;
    // uint8_t new_sym = w->syms + sym;
    // call word_create
    Word *new_w = (Word *) malloc(sizeof(Word));
    if (new_w) {
        // The length of the array of symbols is given by len.
        new_w->syms = (uint8_t *) calloc(new_len, sizeof(uint8_t));
        // copy chars in syms over
        for (uint32_t i = 0; i < w->len; i++) {
            new_w->syms[i] = w->syms[i];
        }
        new_w->syms[new_len - 1] = sym;
        new_w->len = new_len;
    }
    return new_w;
}

// Destructor for a Word, w.
/*
 * Deletes the word w
 * Frees up associated space
 */
void word_delete(Word *w) {
    free(w->syms);
    free(w);
}

// Creates a new WordTable, which is an array of Words.
/*
 * Constructor:
 * Creates a new table big enough to fit MAX_CODE
 * Creates the first element at EMPTY_CODE and returns it
 */
WordTable *wt_create(void) {
    // allocate memory for word table
    // A WordTable has a pre-defined size of MAX_CODE, which has the value UINT16_MAX.
    WordTable *wt = (WordTable *) calloc(MAX_CODE, sizeof(WordTable));
    // call to word create
    // A WordTable is initialized with a single Word at index EMPTY_CODE.
    // represents the empty word, a string of length of zero.
    wt[EMPTY_CODE] = word_create(NULL, 0);
    return wt;
}

// Resets a WordTable, wt, to contain just the empty Word.
/*
 * Deletes all words except EMPTY_CODE
 * Frees associated memory
 */
void wt_reset(WordTable *wt) {
    // Make sure all the other words in the table are NULL.
    for (int i = START_CODE; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            wt[i] = NULL;
        }
    }
}

// Destructor for a Word, w.
/*
 * Destructor: Deletes all words and tables
 * Frees up associated memory
 */
void wt_delete(WordTable *wt) {
    for (int i = EMPTY_CODE; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]);
        }
    }
    free(wt);
}
