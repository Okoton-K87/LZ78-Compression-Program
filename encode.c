#include <stdio.h>
#include <stdlib.h> //atof
#include <unistd.h> //getopt().
#include <fcntl.h> // read open
#include <sys/stat.h>

#include "code.h"
#include "endian.h"
#include "io.h"
#include "trie.h"

#define OPTIONS "i:o:vh"

// this function takes a uint16 and returns its bit length
int bit_len(uint16_t n) {
    int count = 0;
    while (n != 0) {
        count += 1;
        n = n >> 1;
    }
    return count;
}

int main(int argc, char **argv) {
    int opt = 0;

    // disable verbose by default
    int verbose = 0;

    // file descriptors
    int infile_descriptor = STDIN_FILENO;
    int outfile_descriptor = STDOUT_FILENO;

    // default names for files
    char *infile_name = NULL;
    char *outfile_name = NULL;

    // help_message
    const char *help_message
        = "SYNOPSIS\n"
          "   Compresses files using the LZ78 compression algorithm.\n"
          "   Compressed files are decompressed with the corresponding decoder.\n\n"
          "USAG\n"
          "   ./encode [-vh] [-i input] [-o output]\n\n"
          "OPTIONS\n"
          "   -v          Display compression statistics\n"
          "   -i input    Specify input to compress (stdin by default)\n"
          "   -o output   Specify output of compressed input (stdout by default)\n"
          "   -h          Display program help and usage\n";

    // 1. Parse command-line options using getopt() and handle them accordingly.
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile_name = optarg; break;
        case 'o': outfile_name = optarg; break;
        case 'v': verbose = 1; break;
        case 'h': printf("%s", help_message); return 1;
        default: fprintf(stderr, "Usage: %s [-i input] [-o output] [-v] [-h]\n", argv[0]); exit(1);
        }
    }

    // 1. Open infile with open(). If an error occurs, print a helpful message and exit with a status code indicating
    // that an error occurred. infile should be stdin if an input file wasn’t specified.
    if (infile_name != NULL) {
        infile_descriptor = open(infile_name, O_RDONLY);
        if (infile_descriptor == -1) {
            fprintf(stderr, "Error: unable to open input file -- '%s'\n", infile_name);
            exit(1);
        }
    }

    // 2. The first thing in outfile must be the file header, as defined in the file io.h. The magic number in the
    // header must be 0xBAADBAAC. The file size and the protection bit mask you will obtain using fstat(). See
    // the man page on it for details.
    FileHeader infile_header;
    infile_header.magic = 0;
    infile_header.protection = 0;

    infile_header.magic = MAGIC;
    struct stat protection_bits;
    fstat(infile_descriptor, &protection_bits);
    infile_header.protection = protection_bits.st_mode;

    // write_header(infile_descriptor, infile_header);

    // 3. Open outfile using open(). The permissions for outfile should match the protection bits as set in your
    // file header. Any errors with opening outfile should be handled like with infile. outfile should be
    // stdout if an output file wasn’t specified.
    if (outfile_name != NULL) {
        outfile_descriptor = open(outfile_name, O_WRONLY | O_CREAT);
        if (outfile_descriptor == -1) {
            fprintf(stderr, "Error: unable to open output file -- '%s'\n", outfile_name);
            exit(1);
        }
    }
    fchmod(outfile_descriptor, infile_header.protection);

    // struct stat infile_info;
    // fstat(infile_descriptor, &infile_info);

    // struct stat outfile_info;
    // fstat(outfile_descriptor, &outfile_info);
    // 4. Write the filled out file header to outfile using write_header(). This means writing out the struct itself
    // to the file, as described in the comment block of the function.
    write_header(outfile_descriptor, &infile_header);

    // 5. Create a trie. The trie initially has no children and consists solely of the root. The code stored by this root trie
    // node should be EMPTY_CODE to denote the empty word. You will need to make a copy of the root node and
    // use the copy to step through the trie to check for existing prefixes. This root node copy will be referred to as
    // curr_node. The reason a copy is needed is that you will eventually need to reset whatever trie node you’ve
    // stepped to back to the top of the trie, so using a copy lets you use the root node as a base to return to.
    TrieNode *root = trie_create();
    root->code = EMPTY_CODE;
    TrieNode *curr_node;
    curr_node = root;

    // 6. You will need a monotonic counter to keep track of the next available code. This counter should start at
    // START_CODE, as defined in the supplied code.h file. The counter should be a uint16_t since the codes
    // used are unsigned 16-bit integers. This will be referred to as next_code.
    uint16_t next_code = START_CODE;

    // 7. You will also need two variables to keep track of the previous trie node and previously read symbol. We will
    // refer to these as prev_node and prev_sym, respectively.
    TrieNode *prev_node = NULL;
    uint8_t prev_sym = 0;

    // 8. Use read_sym() in a loop to read in all the symbols from infile. Your loop should break when read_sym()
    // returns false. For each symbol read in, call it curr_sym, perform the following:
    uint8_t curr_sym = 0;
    while (read_sym(infile_descriptor, &curr_sym)) {
        // (a) Set next_node to be trie_step(curr_node, curr_sym), stepping down from the current node to
        // the currently read symbol.
        TrieNode *next_node = trie_step(curr_node, curr_sym);

        // (b) If next_node is not NULL, that means we have seen the current prefix. Set prev_node to be curr_node
        // and then curr_node to be next_node.
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            // (c) Else, since next_node is NULL, we know we have not encountered the current prefix. We write the pair
            // (curr_node->code, curr_sym), where the bit-length of the written code is the bit-length of next_code.
            write_pair(outfile_descriptor, curr_node->code, curr_sym, bit_len(next_code));
            // We now add the current prefix to the trie. Let curr_node->children[curr_sym] be a new trie node
            // whose code is next_code.
            curr_node->children[curr_sym] = trie_node_create(next_code);
            // Reset curr_node to point at the root of the trie and increment the value of next_code.
            curr_node = root;
            next_code++;

            // (d) Check if next_code is equal to MAX_CODE. If it is, use trie_reset() to reset the trie to just having the
            // root node. This reset is necessary since we have a finite number of codes.
            if (next_code == MAX_CODE) {
                trie_reset(root);
                curr_node = root;
                next_code = START_CODE;
            }
        }

        // (e) Update prev_sym to be curr_sym.
        prev_sym = curr_sym;
    }
    // 9. After processing all the characters in infile, check if curr_node points to the root trie node. If it does not,
    // it means we were still matching a prefix. Write the pair (prev_node->code, prev_sym). The bit-length of the
    // code written should be the bit-length of next_code. Make sure to increment next_code and that it stays
    // within the limit of MAX_CODE. Hint: use the modulo operator.
    if (curr_node != root) {
        write_pair(outfile_descriptor, prev_node->code, prev_sym, bit_len(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }

    // 10. Write the pair (STOP_CODE, 0) to signal the end of compressed output. Again, the bit-length of code written
    // should be the bit-length of next_code.
    write_pair(outfile_descriptor, STOP_CODE, 0, bit_len(next_code));

    // 11. Make sure to use flush_pairs() to flush any unwritten, buffered pairs. Remember, calls to write_pair()
    // end up buffering them under the hood. So, we have to remember to flush the contents of our buffer.
    flush_pairs(outfile_descriptor);

    if (verbose) {
        // Compressed file size: 25 bytes
        // Uncompressed file size: 15 bytes
        // Compression ratio: -66.67%

        printf("Compressed file size: %lu bytes\n",
            (total_bits / 8 + (total_bits % 8 ? 1 : 0)) + sizeof(FileHeader));
        printf("Uncompressed file size: %lu bytes\n", total_syms);
        double compression_percentage
            = 100.0
              * (1.0
                  - ((double) (total_bits / 8 + (total_bits % 8 ? 1 : 0) + sizeof(FileHeader))
                      / (double) total_syms));
        printf("Space saving: %.2f%%\n", compression_percentage);
    }

    // 12. Use close() to close infile and outfile.
    trie_delete(root);
    close(infile_descriptor);
    close(outfile_descriptor);
    // return 0;
}
