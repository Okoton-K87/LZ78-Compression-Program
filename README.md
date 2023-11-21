# Assignment 6 - LZ78 Compression Program

## Description:
This program provides LZ78 compression and decompression functionality. It includes the following components:

- `encode`: Compresses files using the LZ78 compression algorithm.
- `decrypt`: Decompresses files with the LZ78 decompression algorithm.

## Makefile Usage:
### The following commands will build the encode, decode executable together.
```
make
```
```
make all
```
### The following commands will build the encode, decode executable respectively, along with their required object files.
```
make encode
```
```
make decode
```

### The following command will remove all files that are compiler generated.
```
make clean
```

### The following command will format all source code, including the header files.
```
make format
```


## Running with Command-Line Options

### `encode`
SYNOPSIS
   Compresses files using the LZ78 compression algorithm.
   Compressed files are decompressed with the corresponding decoder.

USAGE
   ./encode1 [-vh] [-i input] [-o output]

OPTIONS
   -v          Display compression statistics
   -i input    Specify input to compress (stdin by default)
   -o output   Specify output of compressed input (stdout by default)
   -h          Display program help and usage


### `decode`
SYNOPSIS
   Decompresses files with the LZ78 decompression algorithm.
   Used with files compressed with the corresponding encoder.

USAGE
   ./decode1 [-vh] [-i input] [-o output]

OPTIONS
   -v          Display decompression statistics
   -i input    Specify input to decompress (stdin by default)
   -o output   Specify output of decompressed input (stdout by default)
   -h          Display program usage


## Citation
Ben Grant, trie.c (trie_node_create, trie_node_delete, trie_reset, trie_delete)
Ben Grant, word.c (word_create, word_append_sym)
Ben Grant, io.c (debug for read/write pairs, read_header - checking magic number and swapping Endianness)
Ben Grant, io.c (debug for read_sym)
Ben Grant, encode.c (fstat - protection bits, verbose output)
Ben Grant, decode.c (verbose output)

Varun, io.c (write_pair, read_pair - (1 << write_pair_buffer_index % 8), adding bitlen + 8)

Ernani, io.c (read_pair - (pair_buffer[read_pair_buffer_index / 8] >> (read_pair_buffer_index % 8) & (uint8_t) 1))
Ernani, io.c (flush_pair)

Lev, io.c (flush_pair - (write_pair_buffer_index % 8 ? 1 : 0))

Miles, encode.c (STDIN_FILENO and STDIN_FILENO)
Miles, io.c (read_sym)

Audery, trie.c (sudocode from group section)
Audery, word.c (sudocode from group section)
Audery, io.c (sudocode from group section)

github, code comments, bv16.h

https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/

https://www.computerhope.com/jargon/f/file-descriptor.htm - Ben Grant





