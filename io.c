#include "word.h"
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> //read write
#include <string.h> // memset

#include "endian.h"
#include "io.h"
#include "code.h"

uint64_t total_syms = 0;
uint64_t total_bits = 0;

uint8_t sym_buffer[BLOCK];

uint8_t pair_buffer[BLOCK];

static int read_pair_buffer_index = 0;
static int write_pair_buffer_index = 0;
static int sym_buffer_index = 0;
static int sym_buffer_index_end = 0;

// #define BLOCK 4096 // 4KB blocks.
// #define MAGIC 0xBAADBAAC // Unique encoder/decoder magic number.

// extern uint64_t total_syms; // To count the symbols processed.
// extern uint64_t total_bits; // To count the bits processed.

// typedef struct FileHeader {
//     uint32_t magic;
//     uint16_t protection;
// } FileHeader;

//
// Read up to to_read bytes from infile and store them in buf. Return the number of bytes actually
// read.
//
// Since read() may not read in as many bytes as you asked for, this function should continuously
// call read() and attempt to read as many bytes as it has not yet read. For instance, if to_read is
// 100 and the first read() call only reads 20 bytes, it should attempt to read 80 bytes the next
// time it calls read().
//
int read_bytes(int infile, uint8_t *buf, int to_read) {
    // if to_read is 0, return 0
    if (to_read == 0) {
        return 0;
    }

    // keep track bytes read, init to 0
    int bytes_read = 0;
    // keep track of bytes read on latest read call to 0
    int current = 0;

    // while (current = read(infile, buffer, to_read - bytes read so far)) {
    //     total bytes_read so far += current
    //     break once reaach to_read amouts of bytes
    // }
    while ((current = read(infile, buf, to_read - bytes_read))) {
        if (current == 0) {
            break;
        }

        bytes_read += current;
        if (bytes_read == to_read) {
            break;
        }
    }

    // return total bytes read
    return bytes_read;
}

//
// Write up to to_write bytes from buf into outfile. Return the number of bytes actually written.
//
// Similarly to read_bytes, this function will need to call write() in a loop to ensure that it
// writes as many bytes as possible.
//
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    // if to_write is 0, return 0
    if (to_write == 0) {
        return 0;
    }

    // keep track bytes wrote, init to 0
    int bytes_wrote = 0;
    // keep track of bytes read on latest read call to 0
    int current = 0;

    // while (current = read(outfile, buffer, to_write - bytes write so far)) {
    //     total bytes_wrote so far += current
    //     break once reaach to_write amouts of bytes
    // }
    while ((current = write(outfile, buf, to_write - bytes_wrote))) {
        bytes_wrote += current;
        if (bytes_wrote == to_write) {
            break;
        }
    }

    // return total bytes read
    return bytes_wrote;
}

//
// Read a file header from infile into *header.
//
// This function need not create any additional buffer to store the contents of the file header.
// File headers, like any struct or any value in C, are just represented by bytes in memory which
// means you can use read_bytes to read however many bytes the header consumes (use sizeof!) into
// *header.
//
// Since we decided that the canonical byte order for our headers is little-endian, this function
// will need to swap the byte order of both the header fields if it is run on a big-endian system.
// For example, here is how the 4 bytes of the magic number will look when written to the file:
//
// +------+------+------+------+
// | 0xAC | 0xBA | 0xAD | 0xBA |
// +------+------+------+------+
//
// A big-endian computer would interpret those bytes as the 4-byte number 0xACBAADBA, which is
// not what we want, so you would have to change the order of those bytes in memory. A little-endian
// computer will interpret that as 0xBAADBAAC.
//
// This function should also make sure the magic number is correct. Since it has no return value you
// may call assert() to do that, or print out an error message and exit the program, or use some
// other way to report the error.
//
void read_header(int infile, FileHeader *header) {
    // This reads in sizeof(FileHeader) bytes from the input file.
    // These bytes are read into the supplied header.
    int bytes_read = read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    if (bytes_read != sizeof(FileHeader)) {
        return;
    }
    // Endianness is swapped if byte order isn’t little endian.
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // Along with reading the header, it must verify the magic number.
    if (header->magic != MAGIC) {
        return;
    }
}

//
// Write a file header from *header to outfile. Like above, this function should swap the byte order
// of the header's two fields if necessary.
//
void write_header(int outfile, FileHeader *header) {
    // Endianness is swapped if byte order isn’t little endian.
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // Writes sizeof(FileHeader) bytes to the output file.
    // These bytes are from the supplied header.
    int bytes_wrote = write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    if (bytes_wrote != sizeof(FileHeader)) {
        return;
    }
}

//
// Read one symbol from infile into *sym. Return true if a symbol was successfully read, false
// otherwise.

// Reading one symbol at a time is slow, so this function will need to maintain a global buffer
// (an array) of BLOCK bytes. Most calls will only need to read a symbol out of that buffer, and
// then update some counter so that the function knows what position in the buffer it is at. If
// there are no more bytes in the buffer for it to return, it will have to call read_bytes to refill
// the buffer with fresh data. If this call fails then you cannot read a symbol and should return false.
// ----------------------------------------------------
// sym_buffer for read_sym, write_word, and flush_words
bool read_sym(int infile, uint8_t *sym) {
    // maintain a global buffer (an array) of BLOCK bytes.
    // uint8_t sym_buffer[BLOCK];
    // An index keeps track of the currently read symbol in the buffer.
    // static int sym_buffer_index = 0;

    // if no more bytes in the buffer
    if (sym_buffer_index >= sym_buffer_index_end) {
        // call read_bytes to refill the buffer with fresh data
        int bytes_read = read_bytes(infile, sym_buffer, BLOCK);
        // If this call fails then you cannot read a symbol and should return false.
        if (bytes_read == 0) {
            return false;
        }
        sym_buffer_index = 0;
        sym_buffer_index_end = bytes_read;
    }
    // Read one symbol from infile into *sym.
    *sym = sym_buffer[sym_buffer_index];
    // update some counter
    sym_buffer_index += 1;
    total_syms += 1;

    return true;
}

//
// Write a pair -- bitlen bits of code, followed by all 8 bits of sym -- to outfile.
//
// This function should also use a buffer. It writes into individual bits in the buffer, starting
// with the least significant bit of the first byte, until the most significant bit of the first
// byte, and then the least significant bit of the second byte, and so on. You will need bitwise
// arithmetic to manipulate individual bits within your buffer -- consult asgn3/set.c if you don't
// remember how to do this.
//
// The first bit of code to be written is the least significant bit, and the same holds for sym.
//
// This function will need to track which *bit* in the buffer will be written to next. If it ever
// reaches the end of the buffer it needs to write out the contents of the buffer to outfile; you
// may use flush_pairs to do this.
// ######################################################
// pair_buffer for read_pair, write_pair, and flush_pairs
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    // uint8_t pair_buffer[BLOCK];
    // pair_buffer_index = 0;
    // “Writes” a pair to outfile. In reality, the pair is buffered.
    // A pair is comprised of a code and a symbol.

    // The bits of the code are buffered first, starting from the LSB.
    for (int i = 0; i < bitlen; i++) {
        // checks if a pair_buffer is full
        if (write_pair_buffer_index == 8 * BLOCK) {
            // write the pair_buffer to oufile
            write_bytes(outfile, pair_buffer, BLOCK);
            memset(pair_buffer, 0, BLOCK); //  Set buffer to all zeros. Varun
            // reset index to 0
            write_pair_buffer_index = 0;
        }
        // checks if the current bit of code is set
        if (code & (1 << i)) {
            // set the corresponding bit in the buffer
            // write_pair_buffer_index / 8 calcuates buffer index
            // write_pair_buffer_index % 8 calcuates the amount to set for specific bit
            pair_buffer[write_pair_buffer_index / 8] |= (1 << write_pair_buffer_index % 8);
        }
        // add 1 to index
        write_pair_buffer_index += 1;
    }
    // The bits of the symbol are buffered next, also starting from the LSB.
    for (int i = 0; i < 8; i++) {
        // checks if a pair_buffer is full
        if (write_pair_buffer_index == 8 * BLOCK) {
            // write the pair_buffer to oufile
            write_bytes(outfile, pair_buffer, BLOCK);
            memset(pair_buffer, 0, BLOCK); //  Set buffer to all zeros. Varun
            // reset index to 0
            write_pair_buffer_index = 0;
        }
        // checks if the current bit of sym is set
        if (sym & (1 << i)) {
            // set the corresponding bit in the buffer
            // write_pair_buffer_index / 8 calcuates buffer index
            // write_pair_buffer_index % 8 calcuates the amount to set for specific bit
            pair_buffer[write_pair_buffer_index / 8] |= (1 << write_pair_buffer_index % 8);
        }
        // add 1 to index
        write_pair_buffer_index += 1;
    }
    // The code buffered has a bit-length of bitlen. The buffer is written out whenever it is filled.
    total_bits += bitlen + 8;
}

//
// Write any pairs that are in write_pair's buffer but haven't been written yet to outfile.
//
// This function will need to be called at the end of encode since otherwise those pairs would never
// be written. You don't have to, but it would be easy to make this function also work when called
// by write_pair, since otherwise you would write essentially the same code in two places.
//
// If not all bits of the last byte in your buffer have been written to, make sure that the
// unwritten bits are set to zero. An easy way to do this is by zeroing the entire buffer after
// flushing it every time.
// ######################################################
// pair_buffer for read_pair, write_pair, and flush_pairs
void flush_pairs(int outfile) {
    // calculate number of bytes needed to write out remaining bits
    int remaining_bytes = write_pair_buffer_index / 8 + (write_pair_buffer_index % 8 ? 1 : 0);

    // write remaining bytes to outfile
    write_bytes(outfile, pair_buffer, remaining_bytes);

    // reset buffer and buffer index
    memset(pair_buffer, 0, BLOCK);
    write_pair_buffer_index = 0;
}

//
// Read bitlen bits of a code into *code, and then a full 8-bit symbol into *sym, from infile.
// Return true if the complete pair was read and false otherwise.
//
// Like write_pair, this function must read the least significant bit of each input byte first, and
// will store those bits into the LSB of *code and of *sym first.
//
// It may be useful to write a helper function that reads a single bit from a file using a buffer.
// ######################################################
// pair_buffer for read_pair, write_pair, and flush_pairs
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    int bytes_read;

    if (read_pair_buffer_index == 0) {
        // reads BLOCK bytes from the input file
        bytes_read = read_bytes(infile, pair_buffer, BLOCK);
        // if no bytes were read
        if (bytes_read == 0) {
            return false;
        }
    }

    *code = 0;
    // The bits of the code are buffered first, starting from the LSB.
    for (int i = 0; i < bitlen; i++) {
        // checks if a pair_buffer is full
        if (read_pair_buffer_index == 8 * BLOCK) {
            // read BLOCK bytes from infile to pair_buffer
            bytes_read = read_bytes(infile, pair_buffer, BLOCK);
            if (bytes_read == 0) {
                return false;
            }
            // reset index
            read_pair_buffer_index = 0;
        }
        // return (x->v[k / BITS_PER_UNIT] >> k % BITS_PER_UNIT) & 0x1;
        // extracts the value of the least significant bit of the shifted byte.
        if (pair_buffer[read_pair_buffer_index / 8] >> (read_pair_buffer_index % 8) & (uint8_t) 1) {
            // sets the ith bit of code to 1
            *code |= (1 << i);
        }
        read_pair_buffer_index += 1;
    }

    *sym = 0;
    // The bits of the symbol are buffered next, also starting from the LSB.
    for (int i = 0; i < 8; i++) {
        if (read_pair_buffer_index == 8 * BLOCK) {
            bytes_read = read_bytes(infile, pair_buffer, BLOCK);
            if (bytes_read == 0) {
                return false;
            }
            read_pair_buffer_index = 0;
        }
        // return (x->v[k / BITS_PER_UNIT] >> k % BITS_PER_UNIT) & 0x1;
        // extracts the value of the least significant bit of the shifted byte.
        if (pair_buffer[read_pair_buffer_index / 8] >> (read_pair_buffer_index % 8) & (uint8_t) 1) {
            // sets the ith bit of sym to 1
            *sym |= (1 << i);
        }
        read_pair_buffer_index += 1;
    }

    // Update the bit counters
    total_bits += bitlen + 8;

    // Returns true if there are pairs left to read in the buffer, else false.
    // There are pairs left to read if the read code is not STOP_CODE.
    return (*code != STOP_CODE);
}

//
// Write every symbol from w into outfile.
//
// These symbols should also be buffered and the buffer flushed whenever necessary (note you will
// likely sometimes fill up your buffer in the middle of writing a word, so you cannot only check
// that the buffer is full at the end of this function).
// ----------------------------------------------------
// sym_buffer for read_sym, write_word, and flush_words
void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        // Each symbol of the Word is placed into a buffer.
        sym_buffer[sym_buffer_index] = w->syms[i];
        sym_buffer_index += 1;
        // The buffer is written out when it is filled.
        if (sym_buffer_index == BLOCK) {
            write_bytes(outfile, sym_buffer, BLOCK);
            sym_buffer_index = 0;
        }
        total_syms += 1;
    }
}

//
// Write any unwritten word symbols from the buffer used by write_word to outfile.
//
// Similarly to flush_pairs, this function must be called at the end of decode since otherwise you
// would have symbols remaining in the buffer that were never written.
// ----------------------------------------------------
// sym_buffer for read_sym, write_word, and flush_words
void flush_words(int outfile) {
    // calculate number of bytes needed to write out remaining symbols
    int remaining_bytes = sym_buffer_index;

    // write remaining bytes to outfile
    write_bytes(outfile, sym_buffer, remaining_bytes);

    // reset buffer and buffer index
    memset(sym_buffer, 0, BLOCK);
    sym_buffer_index = 0;
}
