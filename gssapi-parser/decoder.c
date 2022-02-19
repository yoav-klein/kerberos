
#include <unistd.h> /* close */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "InitialContextToken.h"
#include "NegotiationToken.h"


void usage(char *prog)
{
    printf("Usage: %s filename\n", prog);
}

void print_bytes(unsigned char *buff, ssize_t size)
{
    ssize_t i = 0;
    ssize_t j = 0;
    for(i = 0; i < (size / 16) + 1; ++i)
    {
        for(j = 0; j < ((size >= 16) ? 16 : size); ++j)
        {
            printf("%x ", buff[i * 16 + j]);
        }
        size -= 16;
        printf("\n");
    }
}

int check_if_spnego(OBJECT_IDENTIFIER_t *oid)
{
    /* the actual oid for spnego is 1.3.6.1.5.5.2, but see in RFC 2743
    that the first octet is the (first_number * 40 + second_number)
     */
    uint8_t spnego_oid[] = { 43, 6, 1, 5, 5, 2 }; 
    size_t size = 6;
    size_t i = 0;
    
    if(oid->size != size)
    {
        return 0;
    }
    for(i = 0; i < size; ++i)
    {
        if(spnego_oid[i] != oid->buf[i])
        {
            return 0;
        }
    }
    return 1;

}

void parse_spnego(ANY_t *spnego_payload)
{
    asn_dec_rval_t status;
    NegotiationToken_t *neg_tok = NULL;
    
    status = ber_decode(0, &asn_DEF_NegotiationToken, (void**)&neg_tok, spnego_payload->buf, spnego_payload->size);
    if(status.code != RC_OK)
    {
        fprintf(stderr, "Broken NegotiationToken encoding at byte: %ld\n", status.consumed);
        exit(1);
    }
    
    xer_fprint(stdout, &asn_DEF_NegotiationToken, neg_tok);
    ASN_STRUCT_FREE(asn_DEF_NegotiationToken, neg_tok);
}


int main(int argc, char **argv)
{
    char buff[1024]; /* temporary buffer */
    asn_dec_rval_t status;
    InitialContextToken_t *token = NULL;

    FILE *fp;
    size_t bytes_read = 0;
    char *filename;

    if(argc < 2)
    {
        usage(argv[0]);
        exit(1);
    }
    filename = argv[1];
    
    fp = fopen(filename, "rb");
    if(!fp)
    {
        perror(filename);
        exit(1);
    }

    bytes_read = fread(buff, 1, sizeof(buff), fp);
    fclose(fp);
    if(!bytes_read)
    {
        printf("couldn't read from file\n");
        exit(1);
    }

    /* good for diagnostics */
    /* print_bytes(buff, bytes_read); */

    status = ber_decode(0, &asn_DEF_InitialContextToken, (void**)&token, buff, bytes_read);
    if(status.code != RC_OK)
    {
        fprintf(stderr, "Broken InitialContextToken encoding at byte: %ld\n", status.consumed);
        exit(1);
    }
    
    xer_fprint(stdout, &asn_DEF_InitialContextToken, token);

    if(check_if_spnego((OBJECT_IDENTIFIER_t*)&token->thisMech))
    {
        printf("------------- SPNEGO ----------------- !\n");
        parse_spnego(&token->innerContextToken);
    }

    ASN_STRUCT_FREE(asn_DEF_InitialContextToken, token);
    return 0;

}