#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"

#define MAGIC_STRING "##"

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    return width * height * 3;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
}

/* Open files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
        return e_failure;

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
        return e_failure;

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
        return e_failure;

    return e_success;
}

/* Check capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    uint image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    uint secret_size = get_file_size(encInfo->fptr_secret);

    char *extn = strchr(encInfo->secret_fname, '.');
    int extn_size = strlen(extn);

    uint required_size =
        (strlen(MAGIC_STRING) * 8) +
        32 +
        (extn_size * 8) +
        32 +
        (secret_size * 8);

    if (image_capacity >= required_size)
        return e_success;
    else
        return e_failure;
}

/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_stego_image)
{
    char buffer[54];

    fseek(fptr_src_image, 0, SEEK_SET);
    fread(buffer, 1, 54, fptr_src_image);
    fwrite(buffer, 1, 54, fptr_stego_image);

    return e_success;
}

/* Encode one byte into 8 bytes */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) | ((data >> (7 - i)) & 1);
    }
    return e_success;
}

/* Encode string data */
Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, 1, 8, fptr_src_image);
        encode_byte_to_lsb(data[i], image_buffer);
        fwrite(image_buffer, 1, 8, fptr_stego_image);
    }

    return e_success;
}

/* Encode integer (32 bits) */
Status encode_size_to_lsb(int data,
                          FILE *fptr_src_image,
                          FILE *fptr_stego_image)
{
    char image_buffer[32];

    fread(image_buffer, 1, 32, fptr_src_image);

    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) | ((data >> (31 - i)) & 1);
    }

    fwrite(image_buffer, 1, 32, fptr_stego_image);

    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    return encode_data_to_image(
        (char *)magic_string,
        strlen(magic_string),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image);
}

/* Encode secret file extension */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    char *extn = strchr(encInfo->secret_fname, '.');
    int extn_size = strlen(extn);

    if (encode_size_to_lsb(extn_size,
                           encInfo->fptr_src_image,
                           encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    if (encode_data_to_image(extn,
                             extn_size,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    return encode_size_to_lsb(
        file_size,
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image);
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    rewind(encInfo->fptr_secret);

    while (fread(&ch, 1, 1, encInfo->fptr_secret) > 0)
    {
        if (encode_data_to_image(&ch, 1,
            encInfo->fptr_src_image,
            encInfo->fptr_stego_image) == e_failure)
            return e_failure;
    }

    return e_success;
}

/* Copy remaining image data */
Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char buffer[1024];
    int bytes_read;

    while ((bytes_read =
            fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        fwrite(buffer, 1, bytes_read, fptr_dest);
    }

    return e_success;
}

/* Main encoding controller */
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
        return e_failure;

    if (check_capacity(encInfo) == e_failure)
        return e_failure;

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_extn(encInfo) == e_failure)
        return e_failure;

    long secret_size = get_file_size(encInfo->fptr_secret);

    if (encode_secret_file_size(secret_size, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_data(encInfo) == e_failure)
        return e_failure;

    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    return e_success;
}
#include <string.h>
#include "encode.h"


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
   
    encInfo->src_image_fname   = argv[2];
    encInfo->secret_fname      = argv[3];
    encInfo->stego_image_fname = argv[4];

    return e_success;
}

