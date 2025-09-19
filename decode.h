#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <string.h>
#include "types.h"   

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;                         /* user provided base base name for output */
    FILE *fptr_secret;                          /* output file pointer (opened after ext decoded) */
    int ext_size;                               /* decoded extension length */
    char extn_secret_file[32];   
    char secret_file_name[32]; 
    char user_file_name[32];                    /* decoded extension string (null-terminated) */
    unsigned long secret_file_size;             /* decoded secret file size */

    /* Stego Image Info */
    char *stego_image_fname;                    /* input stego image path */
    FILE *fptr_stego_image;                     /* input file pointer (opened for reading) */

    /* Magic string */
    int magic_string_size;                      /* decoded magic string length */
    char magic_string[64];                      /* decoded magic string */

    /* User-provided magic */
    int user_magic_string_size;                 /* length of user-provided magic */
    char user_magic_string[32];                 /* user-provided magic string */

} DecodeInfo;

unsigned int decode_size_from_lsb(const unsigned char *buffer);
unsigned char decode_byte_from_lsb(const unsigned char *buffer);

/* function prototypes  */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);

/* decode steps: each fills fields inside decInfo */
Status decode_magic_string_len(DecodeInfo *decInfo);    /* fills decInfo->magic_string_size */
Status decode_magic_string(DecodeInfo *decInfo);        /* fills decInfo->magic_string */

Status decode_secret_file_extn_size(DecodeInfo *decInfo);/* fills decInfo->ext_size */
Status decode_secret_file_extn(DecodeInfo *decInfo);    /* fills decInfo->extn_secret_file */

Status decode_secret_file_size(DecodeInfo *decInfo);    /* fills decInfo->secret_file_size */
Status decode_secret_file_data(DecodeInfo *decInfo);    /* writes secret bytes to decInfo->fptr_secret */


Status do_decoding(DecodeInfo *decInfo);

#endif /* DECODE_H */
