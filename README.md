Steganography 
-------------

This project implements simple Least Significant Bit (LSB) based steganography.
It allows hiding a text file inside a BMP image and later extracting it back.
The project is written in C and uses standard file operations.

Compilation
-----------
Use gcc to compile the source files:
gcc *.c

Usage
-----

Encoding:
./a.out -e input.bmp secret.txt stego.bmp
- input.bmp is the cover BMP image.
- secret.txt is the secret file to be hidden.
- stego.bmp is the output file with the secret embedded.
The program will ask for a magic string. Enter any keyword.

Decoding:
./a.out -d stego.bmp output
- stego.bmp is the encoded BMP file.
- output is the optional base name for the recovered file.
If no output name is provided, "output" will be used by default.
The program will ask for the same magic string used during encoding.

Example
-------
echo "Hello World" > secret.txt
./a.out -e input.bmp secret.txt stego.bmp
Enter MAGIC when prompted.
./a.out -d stego.bmp output
Enter MAGIC again.
The file output.txt will be created with the secret.

File Descriptions
-----------------
encode.c    : Functions for encoding
decode.c    : Functions for decoding
encode.h    : Encode function declarations
decode.h    : Decode function declarations
types.h     : Type definitions and enums
common.h    : Shared constants
test_encode.c: Main program

Encoding Process
----------------
1. Open input image, secret file, and stego output file.
2. Check image capacity is sufficient.
3. Copy 54-byte BMP header.
4. Encode magic string length and string.
5. Encode secret file extension length and extension.
6. Encode secret file size.
7. Encode secret file data byte by byte.
8. Copy remaining image data.

Decoding Process
----------------
1. Open stego image and skip 54-byte header.
2. Decode magic string length and string.
3. Compare with user-provided magic string.
4. Decode extension length and extension string.
5. Decode secret file size.
6. Rebuild output file name (base + extension).
7. Decode secret file data and write to disk.

Notes
-----
- Works only with 24-bit BMP images.
- Magic string ensures only valid stego images are decoded.
- Output file name is optional; default is "output".
- Code assumes BMP header size of 54 bytes.
- Capacity check ensures the secret fits in the image.
- This project is for learning purposes, not for secure use.
