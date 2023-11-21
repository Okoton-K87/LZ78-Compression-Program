# LZ78 Compression Program

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
   1. -v          Display compression statistics
   2. -i input    Specify input to compress (stdin by default)
   3. -o output   Specify output of compressed input (stdout by default)
   4. -h          Display program help and usage


### `decode`
SYNOPSIS
   Decompresses files with the LZ78 decompression algorithm.
   Used with files compressed with the corresponding encoder.

USAGE
   ./decode1 [-vh] [-i input] [-o output]

OPTIONS
   1. -v          Display decompression statistics
   2. -i input    Specify input to decompress (stdin by default)
   3. -o output   Specify output of decompressed input (stdout by default)
   4. -h          Display program usage







