#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "trie.h"
#include "code.h"

// struct TrieNode {
//     TrieNode *children[ALPHABET];
//     uint16_t code;
// };

/*
 * Creates a new TrieNode and returns a pointer to it
 * Allocate memory for TrieNode
 * Code is the code to be assigned to this new node
 * Returns the newly allocated node
 */
TrieNode *trie_node_create(uint16_t index) {
    TrieNode *n = (TrieNode *) malloc(sizeof(TrieNode));
    // if allocated
    if (n) {
        // The node’s code is set to code.
        n->code = index;
        //allocate memo for children arr
        // n->children = (TrieNode*)malloc(sizeof(TrieNode));
        for (int i = 0; i < ALPHABET; i++) {
            // Make sure each of the children node pointers are NULL
            n->children[i] = NULL;
        }
    }
    return n;
}

/*
 * Deletes Node n
 * Frees any allocated memory
 */
void trie_node_delete(TrieNode *n) {
    if (n == NULL) {
        return;
    }

    // for (int i = 0; i < ALPHABET; i++) {
    // 	// free non-null children by recursively calling trie_node_delete
    // 	if (n->children[i] != NULL) {
    // 		trie_node_delete(n->children[i]);
    // 	}
    // 	// set freed children to null after
    // 	n->children[i] = NULL;
    // }
    // // free(n->children);
    free(n);
}

/*
 * Constructor: Creates the root TrieNode and returns a pointer to it
 * Allocate memory for TrieNode
 * Code is START_CODE
 * Returns the newly allocated node
 */
TrieNode *trie_create(void) {
    // Initializes a trie: a root TrieNode with the code EMPTY_CODE.
    TrieNode *root = trie_node_create(EMPTY_CODE);
    // Returns the root, a TrieNode *, if successful, NULL otherwise.
    if (root == NULL) {
        return NULL;
    } else {
        return root;
    }
}

/*
 * Resets the trie: called when code reaches MAX_CODE
 * Deletes all the children of root and frees allocated memory
 */
void trie_reset(TrieNode *root) {
    // Since we are working with finite codes,
    // eventually we will arrive at the end of the available codes (MAX_CODE).
    // At that point, we must reset the trie by deleting its children
    // so that we can continue compressing/decompressing the file.
    // Make sure that each of the root’s children nodes are NULL.
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(root->children[i]);
        root->children[i] = NULL;
    }
}

/*
 * Destructor: Deletes all nodes starting at n as the root
 * Deletes any children recursively before deleting the root
 * Frees all the memory allocated for TrieNodes n and below
 */
void trie_delete(TrieNode *n) {
    // This will require recursive calls on each of n’s children.
    if (n == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(n->children[i]);
        n->children[i] = NULL;
    }
    // Make sure to set the pointer to the children nodes to NULL
    // after you free them with trie_node_delete()
    trie_node_delete(n);
    // n = NULL;
}

/*
 * Checks if node has any children called sym
 * Returns the address if found, NULL if absent
 */
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    // If the symbol doesn’t exist, NULL is returned.

    if (n == NULL) {
        return NULL;
    }
    return n->children[sym];
}
