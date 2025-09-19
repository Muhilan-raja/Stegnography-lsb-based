#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"

// int magic_string_len = strlen(encInfo->magic_string);

Status read_and_validate_encode_args(int argc ,char *argv[], EncodeInfo *encInfo)
{
    if (argc < 5) 
    return e_failure; 

    printf("Enter the Magic string\n");
    scanf("%s",encInfo->magic_string);

    if(strstr(argv[2],".bmp")!=NULL)
    {
        printf(".bmp file is present\n");
        encInfo->src_image_fname=argv[2];
    }
    else{
        printf(".bmp is not present\n");
        return e_failure;
    }

    if(strstr(argv[3],".txt")!=NULL)
    {
        printf(".txt file is present\n");
        encInfo->secret_fname=argv[3];
    }
    else{
        printf(".txt is not present\n");
        return e_failure;
    }

    if(strstr(argv[4],".bmp")!=NULL)
    {
        printf("strgo.bmp is present\n");
        encInfo->stego_image_fname=argv[4];
    }
    else{
        printf(".bmp is not present\n");
        return e_failure;
    }

    return e_success;
}
uint get_file_size(FILE *fptr)
{
    if (!fptr) 
    return 0;
    if (fseek(fptr, 0, SEEK_END) != 0) 
    return 0;
    long s = ftell(fptr);
    if (s < 0) 
    { 
        rewind(fptr); 
        return 0; 
    }
    rewind(fptr);
    return (uint)s;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling 
      
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{


    /* Step 1: Get image capacity (in bytes) */
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Step 2: Get secret file size (in bytes) */
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    /* Step 3: Extract secret file extension (e.g., ".txt") */
    char *dot = strrchr(encInfo->secret_fname, '.');
    if (dot)
    {
        strncpy(encInfo->extn_secret_file, dot, sizeof(encInfo->extn_secret_file) - 1);
        encInfo->extn_secret_file[sizeof(encInfo->extn_secret_file) - 1] = '\0';
    }
    else
    {
        encInfo->extn_secret_file[0] = '\0';
    }

    /* Step 4: Calculate number of bytes needed */
    unsigned long required_bytes = 0;
    required_bytes += 4;  // for magic string length
    required_bytes += strlen(encInfo->magic_string); // magic string itself
    required_bytes += 4;  // for extension length
    required_bytes += strlen(encInfo->extn_secret_file); // extension chars
    required_bytes += 4;  // for secret file size
    required_bytes += encInfo->size_secret_file; // actual secret file data

    /* Step 5: Convert to bits */
    unsigned long required_bits = required_bytes * 8;

    /* Step 6: Available bits in image */
    unsigned long available_bits = encInfo->image_capacity * 8;

    /* Step 7: Compare */
    if (available_bits < required_bits)
    {
        return e_failure;
    }

    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    unsigned char header[54];
    if (fseek(fptr_src_image, 0, SEEK_SET) != 0)
    {
        return e_failure;
    }
    
    if (fread(header, 1, 54, fptr_src_image) != 54) 
    {
        return e_failure;
    }
    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    } 
    return e_success;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1],"-d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Starting encoding process...\n");

    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR: Opening files failed!\n");
        return e_failure;
    }
    printf("INFO: Opened files successfully.\n");

    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Image does not have enough capacity to store secret.\n");
        return e_failure;
    }
    printf("INFO: Checked capacity successfully.\n");

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Copying BMP header failed.\n");
        return e_failure;
    }
    printf("INFO: BMP header copied successfully.\n");

    // /**/ NEW STEP: Encode magic string length */
    if (encode_magic_string_len((int)strlen(encInfo->magic_string), encInfo) == e_failure)
    {
        printf("ERROR: Encoding magic string length failed.\n");
        return e_failure;
    }
    printf("INFO: Magic string length encoded successfully.\n");

    /* Now encode the magic string itself */
    if (encode_magic_string(encInfo->magic_string, encInfo) == e_failure)
    {
        printf("ERROR: Encoding magic string failed.\n");
        return e_failure;
    }
    printf("INFO: Magic string encoded successfully.\n");

    int extn_len = strlen(encInfo->extn_secret_file);
    if (encode_secret_file_extn_size(extn_len, encInfo) == e_failure)
    {
        printf("ERROR: Encoding file extension size failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension size encoded successfully.\n");

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: Encoding file extension failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension encoded successfully.\n");

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: Encoding secret file size failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file size encoded successfully.\n");

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Encoding secret file data failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file data encoded successfully.\n");

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Copying remaining image data failed.\n");
        return e_failure;
    }
    printf("INFO: Remaining image data copied successfully.\n");

    printf("INFO: Stego image stored as %s\n", encInfo->stego_image_fname);

    return e_success;
}

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(unsigned char data, unsigned char *image_buffer)
{
    if (!image_buffer)
    {
        return e_failure;
    }

    int i = 0;
    int n = 7;   // start with MSB (bit 7)

    while (i < 8)
    {
        // 1. create mask to fetch nth bit
        unsigned char mask = 1 << n;

        // 2. fetch nth bit from data
        unsigned char bit = data & mask;

        // 3. clear lsb of ith index of image_buffer
        image_buffer[i] = image_buffer[i] & 0xFE;

        // 4. right shift bit to lsb position
        bit = bit >> n;

        // 5. write fetched bit into lsb
        image_buffer[i] = image_buffer[i] | bit;

        // 6. decrement n, increment i
        n--;
        i++;
    }

    return e_success;
}

/* Write a 32-bit unsigned int into 32 image bytes' LSBs (MSB first) */
Status encode_size_to_lsb(unsigned int data, unsigned char *image_buffer)
{
    if (!image_buffer)
    {
        return e_failure;
    }

    int i = 0;
    int n = 31;   // start with MSB (bit 31)

    while (i < 32)
    {
        // 1. create mask to fetch nth bit
        unsigned int mask = 1 << n;

        // 2. fetch nth bit from data
        unsigned int bit = data & mask;

        // 3. clear lsb of ith index of image_buffer
        image_buffer[i] = image_buffer[i] & 0xFE;

        // 4. right shift bit to lsb position
        bit = bit >> n;

        // 5. write fetched bit into lsb
        image_buffer[i] = image_buffer[i] | bit;

        // 6. decrement n, increment i
        n--;
        i++;
    }

    return e_success;
}



/* encode length of magic string as one byte (using encode_byte_to_lsb) */
Status encode_magic_string_len(int len, EncodeInfo *encInfo)
{
    unsigned char buf[32];
    if (fread(buf, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    } 
    if (encode_size_to_lsb(len, buf) != e_success) 
    {
        return e_failure;
    }
    
    if (fwrite(buf, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    
    return e_success;
}


Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    unsigned int n =strlen(magic_string);
    for (int i = 0; i < n; ++i)
    {
        unsigned char buffer[8];
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }
        
        if (encode_byte_to_lsb(magic_string[i], buffer) != e_success) 
        {
            return e_failure;
        }
        
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8) 
        {
            return e_failure;
        }
    }
    return e_success;
}
/* Encode extension length (32 bits -> 32 image bytes LSBs) */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    if (!encInfo) 
    return e_failure;

    unsigned char buf[32];
    /* read 32 image bytes from source */
    if (fread(buf, 1, 32, encInfo->fptr_src_image) != 32) return e_failure;
    /* encode the 32-bit size into those 32 bytes' LSBs */
    if (encode_size_to_lsb((unsigned int)size, buf) != e_success) return e_failure;
    /* write the modified 32 bytes to stego image */
    if (fwrite(buf, 1, 32, encInfo->fptr_stego_image) != 32) return e_failure;

    return e_success;
}

/* Encode secret file size (32 bits -> 32 image bytes LSBs) */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    unsigned char buf[32];
    if (fread(buf, 1, 32, encInfo->fptr_src_image) != 32) 
    return e_failure;
    if (encode_size_to_lsb((unsigned int)file_size, buf) != e_success) 
    return e_failure;
    if (fwrite(buf, 1, 32, encInfo->fptr_stego_image) != 32) 
    return e_failure;

    return e_success;
}


/* Encode secret file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if (!file_extn) 
    return e_success;
    int n = strlen(file_extn);
    unsigned char buf[8];
    for (int i = 0; i < n; ++i)
    {
        if (fread(buf, 1, 8, encInfo->fptr_src_image) != 8) 
        return e_failure;
        if (encode_byte_to_lsb(file_extn[i], buf) != e_success) 
        return e_failure;
        if (fwrite(buf, 1, 8, encInfo->fptr_stego_image) != 8) 
        return e_failure;
    }
    return e_success;
}



Status encode_secret_file_data(EncodeInfo *encInfo)
{
    int ch;
    char imgbuf[8];
    while ((ch = fgetc(encInfo->fptr_secret)) != EOF)
    {
        if (fread(imgbuf, 1, 8, encInfo->fptr_src_image) != 8) 
        return e_failure;
        if (encode_byte_to_lsb((char)ch, imgbuf) != e_success) 
        return e_failure;
        if (fwrite(imgbuf, 1, 8, encInfo->fptr_stego_image) != 8) 
        return e_failure;
    }
    return e_success;
}


Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    unsigned char buffer[256];

    int n;

    while ((n = fread(buffer, 1, 256, fptr_src)) > 0)
    {
        fwrite(buffer, 1, n, fptr_dest);
    }

    return e_success;
}

