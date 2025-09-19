#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"


unsigned char decode_byte_from_lsb(const unsigned char *buffer)
{
    unsigned char value = 0;

    for (int i = 0; i < 8; ++i)
    {
        value = (value << 1) | (buffer[i] & 1);
    }


    return value;
}

unsigned int decode_size_from_lsb(const unsigned char *buffer)
{
    unsigned int value = 0;

    for (int i = 0; i < 32; ++i)
    {
        value = (value << 1) | (buffer[i] & 1);
    }


    return value;
}

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3)   
        return d_failure; 

    printf("Enter the Magic string\n");
    scanf("%31s", decInfo->user_magic_string);
    
    decInfo->user_magic_string_size = strlen(decInfo->user_magic_string);

    if (strstr(argv[2], ".bmp") != NULL)
    {
        printf(".bmp file is present\n");
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf(".bmp is not present\n");
        return d_failure;
    }

    // if output name given, use it; otherwise default to "output"
    if (argc >= 4)
        decInfo->secret_fname = argv[3];
    else
        decInfo->secret_fname = "output";

    return d_success;
}


Status open_decode_files(DecodeInfo *decInfo)
{
    // Src Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"rb");
    // Do Error handling 
      
    if (decInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n",decInfo->stego_image_fname);

    	return d_failure;
    }

    return d_success;
}


Status decode_magic_string_len(DecodeInfo *decInfo)
{
    unsigned char buf[32];

    if (fread(buf, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return d_failure;
    }

    decInfo->magic_string_size = decode_size_from_lsb(buf);
    return d_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->magic_string_size; i++)
    {
        unsigned char buf[8];
        if (fread(buf, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return d_failure;
        }

        decInfo->magic_string[i] =decode_byte_from_lsb(buf);
    }

    decInfo->magic_string[decInfo->magic_string_size] = '\0';  // null-terminate
    return d_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char buf[32];

    if (fread(buf, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return d_failure;
    }

    decInfo->ext_size = decode_size_from_lsb(buf);
    return d_success;
}


Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->ext_size; i++)
    {
        unsigned char buf[8];
        if (fread(buf, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return d_failure;
        }

        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buf);
    }

    decInfo->extn_secret_file[decInfo->ext_size] = '\0';  // null terminate

    return d_success;
}


Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return d_failure;

    decInfo->secret_file_size = decode_size_from_lsb(buffer);

    return d_success;
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        unsigned char buffer[8];
        unsigned char output;

        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return d_failure;

        output = decode_byte_from_lsb(buffer); 

        if (fwrite(&output, 1, 1, decInfo->fptr_secret) != 1)
            return d_failure;
    }

    return d_success;
}


Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: Starting decoding process...\n");

    if (open_decode_files(decInfo) == d_failure)
    {
        printf("ERROR: Opening files failed!\n");
        return d_failure;
    }
    printf("INFO: Opened files successfully.\n");

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // skip BMP header

    if (decode_magic_string_len(decInfo) == d_failure)
    {
        printf("ERROR: Decoding magic string length failed.\n");
        return d_failure;
    }
    printf("INFO: Magic string length decoded: %d\n", decInfo->magic_string_size);

    if (decode_magic_string(decInfo) == d_failure)
    {
        printf("ERROR: Decoding magic string failed.\n");
        return d_failure;
    }

    printf("INFO: Decoded magic string: %s\n", decInfo->magic_string);

    /* Compare with user-provided magic from the user entered to the magic string in the file */
    if (strcmp(decInfo->magic_string, decInfo->user_magic_string) != 0)
    {
        printf("ERROR: Magic string mismatch!\n");
        return d_failure;
    }

    if (decode_secret_file_extn_size(decInfo) == d_failure)
    {
        printf("ERROR: Decoding file extension size failed.\n");
        return d_failure;
    }
    printf("INFO: Extension size decoded.\n");

    if (decode_secret_file_extn(decInfo) == d_failure)
    {
        printf("ERROR: Decoding file extension failed.\n");
        return d_failure;
    }
    printf("INFO: Extension decoded.\n");

    unsigned int secret_size;
    if (decode_secret_file_size(decInfo) == d_failure)
    {
        printf("ERROR: Decoding secret file size failed.\n");
        return d_failure;
    }
    printf("INFO: Secret file size decoded.\n");

    // merging the secret file name and the secret file extn

    strcpy(decInfo->secret_file_name, decInfo->secret_fname);
    strcat(decInfo->secret_file_name, decInfo->extn_secret_file);


    // open file for the decode here

    // Secret file
    decInfo->fptr_secret = fopen(decInfo->secret_file_name, "wb");

    // Do Error handling

    if (decInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_file_name);

    	return d_failure;
    }
    // No failure return d_success

    if (decode_secret_file_data(decInfo) == d_failure)
    {
        printf("ERROR: Decoding secret file data failed.\n");
        return d_failure;
    }

    printf("INFO: Secret file data decoded successfully.\n");

    printf("INFO: Decoding finished. Secret saved to %s\n", decInfo->secret_fname);
    return d_success;
}
