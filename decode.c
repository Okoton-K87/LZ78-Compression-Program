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
          "   Decompresses files with the LZ78 decompression algorithm.\n"
          "   Used with files compressed with the corresponding encoder.\n"
          "\n"
          "USAGE\n"
          "   ./decode [-vh] [-i input] [-o output]\n"
          "\n"
          "OPTIONS\n"
          "   -v          Display decompression statistics\n"
          "   -i input    Specify input to decompress (stdin by default)\n"
          "   -o output   Specify output of decompressed input (stdout by default)\n"
          "   -h          Display program usage\n";

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

    // 2. Read in the file header with read_header(), which also verifies the magic number. If the magic number is
    // verified then decompression is good to go and you now have a header which contains the original protection
    // bit mask.
    FileHeader infile_header;
    read_header(infile_descriptor, &infile_header);

    // 3. Open outfile using open(). The permissions for outfile should match the protection bits as set in
    // your file header that you just read. Any errors with opening outfile should be handled like with infile.
    // outfile should be stdout if an output file wasn’t specified.
    if (outfile_name != NULL) {
        outfile_descriptor = open(outfile_name, O_WRONLY | O_CREAT);
        if (outfile_descriptor == -1) {
            fprintf(stderr, "Error: unable to open output file -- '%s'\n", outfile_name);
            exit(1);
        }
    }
    fchmod(outfile_descriptor, infile_header.protection);

    // 4. Create a new word table with wt_create() and make sure each of its entries are set to NULL. Initialize the
    // table to have just the empty word, a word of length 0, at the index EMPTY_CODE. We will refer to this table as
    // table.
    WordTable *table = wt_create();

    // 5. You will need two uint16_t to keep track of the current code and next code. These will be referred to as
    // curr_code and next_code, respectively. next_code should be initialized as START_CODE and functions
    // exactly the same as the monotonic counter used during compression, which was also called next_code.
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    // 6. Use read_pair() in a loop to read all the pairs from infile. We will refer to the code and symbol from each
    // read pair as curr_code and curr_sym, respectively. The bit-length of the code to read is the bit-length of
    // next_code. The loop breaks when the code read is STOP_CODE. For each read pair, perform the following:
    //     (a) As seen in the decompression example, we will need to append the read symbol with the word de-
    //     noted by the read code and add the result to table at the index next_code. The word denoted by the
    //     read code is stored in table[curr_code]. We will append table[curr_code] and curr_sym using
    //     word_append_sym().
    //     (b) Write the word that we just constructed and added to the table with write_word(). This word should
    //     have been stored in table[next_code].
    //     (c) Increment next_code and check if it equals MAX_CODE. If it has, reset the table using wt_reset() and
    // set next_code to be START_CODE. This mimics the resetting of the trie during compression.

    uint8_t curr_sym = 0;
    while (read_pair(infile_descriptor, &curr_code, &curr_sym, bit_len(next_code))) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile_descriptor, table[next_code]);
        next_code += 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }

    // 7. Flush any buffered words using flush_words(). Like with write_pair(), write_word() buffers words
    // under the hood, so we have to remember to flush the contents of our buffer.
    flush_words(outfile_descriptor);

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

    // 8. Close infile and outfile with close().
    wt_delete(table);
    close(infile_descriptor);
    close(outfile_descriptor);
}
