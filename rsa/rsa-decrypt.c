#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <stdio.h>
#include <string.h>

const size_t KEY_LENGTH = 32;
const size_t IV_LENGTH = 12;
const size_t AUTH_TAG_LENGTH = 16;

int decrypt(FILE* in_file, FILE* out_file, FILE* pkey_file, FILE* error_stream);

int main(int argc, char** argv) {
    int exit_code = 0;

    FILE* in_file   = NULL;
    FILE* out_file  = NULL;
    FILE* pkey_file = NULL;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s INPUT_FILE OUTPUT_FILE KEYPAIR_FILE\n", argv[0]);
        exit_code = 1;
        goto cleanup;
    }

    const char* in_fname   = argv[1];
    const char* out_fname  = argv[2];
    const char* pkey_fname = argv[3];

    in_file = fopen(in_fname, "rb");
    if (!in_file) {
        fprintf(stderr, "Could not open input file \"%s\"\n", in_fname);
        goto failure;
    }

    out_file = fopen(out_fname, "wb");
    if (!out_file) {
        fprintf(stderr, "Could not open output file \"%s\"\n", out_fname);
        goto failure;
    }

    pkey_file = fopen(pkey_fname, "rb");
    if (!pkey_file) {
        fprintf(stderr, "Could not open keypair file \"%s\"\n", pkey_fname);
        goto failure;
    }

    int err = decrypt(in_file, out_file, pkey_file, stderr);
    if (err) {
        fprintf(stderr, "Decryption failed\n");
        goto failure;
    }

    fprintf(stderr, "Decryption succeeded\n");
    goto cleanup;

failure:
    exit_code = 1;
cleanup:
    if (pkey_file)
        fclose(pkey_file);
    if (out_file)
        fclose(out_file);
    if (in_file)
        fclose(in_file);

    return exit_code;
}

int decrypt(FILE* in_file, FILE* out_file, FILE* pkey_file, FILE* error_stream) {
    int exit_code = 0;
    int err = 1;

    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = NULL;

    unsigned char* in_buf  = NULL;
    unsigned char* out_buf = NULL;

    ERR_clear_error();

    pkey = PEM_read_PrivateKey(pkey_file, NULL, NULL, NULL);
    if (!pkey) {
        if (error_stream)
            fprintf(error_stream, "Could not load keypair\n");
        goto failure;
    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

    err = EVP_PKEY_decrypt_init(ctx);
    err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    size_t pkey_size = EVP_PKEY_get_size(pkey);
    in_buf  = malloc(pkey_size);
    out_buf = malloc(pkey_size);

    size_t in_nbytes = fread(in_buf, 1, pkey_size, in_file);
    size_t out_nbytes = pkey_size;
    err = EVP_PKEY_decrypt(ctx, out_buf, &out_nbytes, in_buf, in_nbytes);
    fwrite(out_buf, 1, out_nbytes, out_file);

    if (ferror(in_file) || ferror(out_file) || ferror(pkey_file)) {
        if (error_stream)
            fprintf(error_stream, "I/O error\n");
        goto failure;
    }

    if (err <= 0) {
        if (error_stream)
            fprintf(error_stream, "EVP_API error\n");
        goto failure;
    }

    goto cleanup;

failure:
    exit_code = 1;
cleanup:
    free(out_buf);
    free(in_buf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (ERR_peek_error()) {
        exit_code = 1;
        if (error_stream) {
            fprintf(error_stream, "Errors from the OpenSSL error queue:\n");
            ERR_print_errors_fp(error_stream);
        }
    }

    return exit_code;
}