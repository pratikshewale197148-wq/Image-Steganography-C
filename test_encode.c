#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

/* function prototype */
OperationType check_operation_type(char *argv[]);

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage:\n");
        printf("./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("./a.out -d stego.bmp output.txt\n");
        return 0;
    }

    OperationType op = check_operation_type(argv);

    /* -------- ENCODING -------- */
    if(op == e_encode)
    {
        printf("Encoding selected\n");

        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Arguments OK\n");

            if(do_encoding(&encInfo) == e_success)
                printf("Encoding completed successfully\n");
            else
                printf("Encoding failed\n");
        }
        else
        {
            printf("Invalid encode arguments\n");
        }
    }

    /* -------- DECODING -------- */
    else if(op == e_decode)
    {
        printf("Decoding selected\n");

        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if(do_decoding(&decInfo) == e_success)
                printf("Decoding completed successfully\n");
            else
                printf("Decoding failed\n");
        }
        else
        {
            printf("Invalid decode arguments\n");
        }
    }

    /* -------- INVALID OPTION -------- */
    else
    {
        printf("Invalid option\n");
        printf("Use:\n");
        printf("./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("./a.out -d stego.bmp output.txt\n");
    }

    return 0;
}

/* option checker */
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
        return e_encode;

    if(strcmp(argv[1], "-d") == 0)
        return e_decode;

    return e_unsupported;
}
