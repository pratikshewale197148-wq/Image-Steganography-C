#include <stdio.h>
#include <string.h>
#include "decode.h"

#define MAGIC_STRING "##"

/* extract one byte */
char decode_byte_from_lsb(char *buffer)
{
    char data = 0;
    for(int i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= (buffer[i] & 1);
    }
    return data;
}

/* extract 32-bit integer */
int decode_size(FILE *fptr)
{
    char buffer[32];
    int size = 0;

    fread(buffer, 1, 32, fptr);

    for(int i = 0; i < 32; i++)
    {
        size <<= 1;
        size |= (buffer[i] & 1);
    }
    return size;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    decInfo->stego_image_fname = argv[2];
    decInfo->output_fname = argv[3];
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    char buffer[8];

    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if(!decInfo->fptr_stego_image) return e_failure;

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if(!decInfo->fptr_output) return e_failure;

   
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // 1️read magic string 
    char magic[3];
    for(int i=0;i<2;i++)
    {
        fread(buffer,1,8,decInfo->fptr_stego_image);
        magic[i] = decode_byte_from_lsb(buffer);
    }
    magic[2]='\0';

    if(strcmp(magic,MAGIC_STRING)!=0)
    {
        printf("Magic string mismatch\n");
        return e_failure;
    }

    // read extension size 
    int ext_size = decode_size(decInfo->fptr_stego_image);

    // skip extension text 
    for(int i=0;i<ext_size;i++)
        fread(buffer,1,8,decInfo->fptr_stego_image);

    // read secret size 
    int secret_size = decode_size(decInfo->fptr_stego_image);

    // read secret data 
    for(int i=0;i<secret_size;i++)
    {
        fread(buffer,1,8,decInfo->fptr_stego_image);
        char ch = decode_byte_from_lsb(buffer);
        fputc(ch,decInfo->fptr_output);
    }

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}
