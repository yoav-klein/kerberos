
#include <stdio.h> /* perror */
#include <arpa/inet.h> /* htonl */
#include <gssapi.h> /* gss_buffer_desc */
#include <unistd.h> /* read */
#include <stdlib.h> /* malloc */
#include <gssapi.h> /* GSS_C_DELEG_FLAG */


/*****************************
*
*   display_context_flags
*
*   takes a flags integer and displays the flags set in it
*
********************************/
void display_context_flags(int ret_flags)
{
    if(ret_flags & GSS_C_DELEG_FLAG)
    {
        printf("context flag: GSS_C_DELEG_FLAG\n");
    }
    if(ret_flags & GSS_C_MUTUAL_FLAG)
    {
        printf("context flag: GSS_C_MUTUAL_FLAG\n");
    }
    if(ret_flags & GSS_C_REPLAY_FLAG)
    {
        printf("context flag: GSS_C_REPLAY_FLAG\n");
    }
    if(ret_flags & GSS_C_SEQUENCE_FLAG)
    {
        printf("context flag: GSS_C_SEQUENCE_FLAG\n");
    }
    if(ret_flags & GSS_C_CONF_FLAG)
    {
        printf("context flag: GSS_C_CONF_FLAG\n");
    }
    if(ret_flags & GSS_C_INTEG_FLAG)
    {
        printf("context flag: GSS_C_INTEG_FLAG\n");
    }
    
}


void display_status_aux(char *msg, OM_uint32 status_val, int status_type)
{
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc status_message;
    unsigned int should_continue = 0;

    do
    {
        maj_stat = gss_display_status(&min_stat, status_val,
            status_type, /* major or minor (GSS or MECH) */
            GSS_C_NO_OID,
            &should_continue, /* outputs whether or not there are more to extract */
            &status_message /* outputs the status message */
        );
        if(GSS_S_COMPLETE != maj_stat)
        {
            printf("gss_display_status failed ! %s\n", msg);
            return;
        }
        if(status_message.length > 0)
        {
            printf("GSS-API ERROR: %s: %s", msg, (char*)status_message.value);
        }
        gss_release_buffer(&min_stat, &status_message);
    }
    while(should_continue);
}

/**************************************
*
*   display_status
*
*   takes a major and minor status and prints
*   an informative message
*
*
***************************************/
void display_status(char *msg, OM_uint32 maj_stat, OM_uint32 min_stat)
{
    display_status_aux(msg, maj_stat, GSS_C_GSS_CODE);
    display_status_aux(msg, min_stat, GSS_C_MECH_CODE);
}


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

    return 0;
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

    token->length = ntohl(*(uint32_t*)&length_buffer);
    token->value = (char*)malloc(token->length);
    status = read_all(fd, token->value, token->length);
    if(-1 == status)
    {
        printf("recv_token: couldn't send token length");
        return -1;
    }
    return 0;
}