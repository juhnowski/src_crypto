#include <openssl/ssl.h>
#include <stdio.h>
int main() {
    printf("Init OpenSSL \n");
    OPENSSL_init_ssl(0, NULL);
    printf("cleaning OpenSSL \n");
    OPENSSL_cleanup();
    printf("Ready \n");
    return 0;
}