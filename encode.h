#ifndef ENCODE_H
#define ENCODE_H

#include "common.h"
#include "types.h"   
#include <stdio.h>

/* Buffer configuration */
#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    char image_data[MAX_IMAGE_BUF_SIZE];
    char magic_string[20]; 
    // char magic_string[4]; 

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    int ext_size;
    char extn_secret_file[32];
    char secret_data[MAX_SECRET_BUF_SIZE];
    unsigned long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;

/* --- Utility prototypes (used by encode.c) --- */
uint get_image_size_for_bmp(FILE *fptr_image);
uint get_file_size(FILE *fptr);

/* --- Argument parsing --- */
/* main must pass argc, argv */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo);

/* --- Encoding pipeline --- */
Status open_files(EncodeInfo *encInfo);
Status check_capacity(EncodeInfo *encInfo);
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* encoding helpers */
Status encode_magic_string_len(int len, EncodeInfo *encInfo);
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* secret-file extension and size */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo);
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* secret data + copying functions */
Status encode_secret_file_data(EncodeInfo *encInfo);
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

/* bit helpers: note unsigned int for size */
Status encode_byte_to_lsb(unsigned char data, unsigned char *image_buffer);
Status encode_size_to_lsb(unsigned int data, unsigned char *image_buffer);

/* top-level */
Status do_encoding(EncodeInfo *encInfo);
OperationType check_operation_type(char *argv[]);

#endif /* ENCODE_H */
