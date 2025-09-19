#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    
    if (argc < 2) 
    {
        printf("==== Usage error ====\n");
        printf("For encode \n");
        fprintf(stderr, "Usage: %s -e <src.bmp> <secret.txt> <stego.bmp>\n", argv[0]);
        printf("For decode\n");
        fprintf(stderr, "Usage: %s -d <stego.bmp> <output file>\n", argv[0]);
        printf("==== xxxx ====\n");

        return 1;
    }

    OperationType op = check_operation_type(argv);

    if (op == e_encode)
    {
        printf("You have chosen encoding\n");

        EncodeInfo encInfo;
        memset(&encInfo, 0, sizeof(encInfo));

        if (read_and_validate_encode_args(argc, argv, &encInfo)!= e_success) {
            fprintf(stderr, "Invalid arguments for encoding\n");
            return 1;
        }

        if (open_files(&encInfo) != e_success) {
            fprintf(stderr, "Error opening files\n");
            return 1;
        } else {
            printf("Files opened successfully\n");
        }

        /* compute image capacity (example: read from BMP header) */
        encInfo.image_capacity = get_image_size_for_bmp(encInfo.fptr_src_image);

        if (check_capacity(&encInfo) == e_success) {
            printf("Image size is enough to encode\n");
        } else {
            printf("Image size is not enough to encode\n");
        }

        if((do_encoding(&encInfo)) == e_success)
        {
            printf("Encoding is successfull\n");
            return e_success;
        }
        else
        {
            printf("Encoding is failed\n");
            return e_failure;
        }



            if (encInfo.fptr_src_image)
                fclose(encInfo.fptr_src_image);
            if (encInfo.fptr_secret)
                fclose(encInfo.fptr_secret);
            if (encInfo.fptr_stego_image)
                fclose(encInfo.fptr_stego_image);

    }
    else if (op == e_decode)
    {
        printf("You have chosen decoding\n");

        DecodeInfo decInfo;
        memset(&decInfo, 0, sizeof(decInfo));

        if (read_and_validate_decode_args(argc, argv, &decInfo)!= d_success) 
        {
            fprintf(stderr, "Invalid arguments for encoding\n");
            return 1;
        }

        if (open_decode_files(&decInfo) != d_success) 
        {
            fprintf(stderr, "Error opening files\n");
            return 1;
        } 
        else 
        {
            printf("Files opened successfully\n");
        }

        if((do_decoding(&decInfo)) == e_success)
        {
            printf("Decoding is successfull\n");
            return e_success;
        }
        else
        {
            printf("Decoding is failed\n");
            return e_failure;
        }
        
        if (decInfo.fptr_stego_image)
            fclose(decInfo.fptr_stego_image);
        if (decInfo.fptr_secret)
            fclose(decInfo.fptr_secret);

        return 0;
    }
    else
    {
        printf("Enter correct argument (-e or -d)\n");
        return 1;
    }
}
