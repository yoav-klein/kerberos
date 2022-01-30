
#include <stdio.h> /* perror */
#include <arpa/inet.h> /* htonl */
#include <gssapi.h> /* gss_buffer_desc */

static int read_all(int fd, char *buffer, size_t len)
{
    int bytes_read = 0;

    while(len)
    {
        bytes_read = read(fd, buffer + bytes_read, len);
        if(-1 == bytes_read)
        {
            perror("read_all");
            return -1;
        }
        len -= bytes_read;
    }
    return 0;
}

static int write_all(int fd, const char *buffer, size_t len)
{
    int bytes_written = 0;

    while(len)
    {
        bytes_written = write(fd, buffer + bytes_written, len);
        if(-1 == bytes_written)
        {
            perror("write_all");
            return -1;
        }
        len -= bytes_written;
    }

    return 0;
}


/******************************
    send_token

    sends the length of the token and then the token itself 

**********************************/

int send_token(int fd, gss_buffer_desc *token)
{
    int status = 0;
    uint32_t len = htonl(token->length);
    status = write_all(fd, (char*)&len, sizeof(uint32_t));
    if(-1 == status)
    {
        printf("send_token: couldn't send token length");
        return -1;
    }

    status = write_all(fd, token->value, token->length);
    if(-1 == status)
    {
        printf("send_token: couldn't send token value");
        return -1;
    }

}

/****************
    recv_token

    takes a pointer to gss_buffer_desc and reads into it a token from peer.
    first, the length is read, and then the contents.

    allocates memory which must be released afterwards

*******************/
int recv_token(int fd, gss_buffer_desc *token)
{
    int status = 0;

    char length_buffer[4];
    status = read_all(fd, length_buffer, sizeof(uint32_t));
    if(-1 == status)
    {
        printf("recv_token: couldn't read token length");
        return -1;
    }

    token->length = ntohl((uint32_t)length_buffer);
    token->value = (char*)malloc(token->length);
    status = read_all(fd, token->value, token->length);
    if(-1 == status)
    {
        printf("recv_token: couldn't send token length");
        return -1;
    }

}