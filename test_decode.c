#include <stdio.h>
#include "decode.h"

int main(int argc, char *argv[])
{
    /* check command line argument */
    if (argc < 2)
    {
        printf("Usage: %s <stego_image.bmp>\n", argv[0]);
        return 1;
    }

    DecodeInfo decInfo;

    /* store stego image name */
    decInfo.stego_image_fname = argv[1];

    /* call decoding */
    if (do_decoding(&decInfo) == e_success)
    {
        printf("Decoding completed successfully\n");
    }
    else
    {
        printf("Decoding failed\n");
    }

    return 0;
}
