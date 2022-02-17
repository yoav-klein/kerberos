
#include <stdio.h> /* perror */
#include <arpa/inet.h> /* htonl */
#include <gssapi.h> /* gss_buffer_desc */
#include <unistd.h> /* read */
#include <stdlib.h> /* malloc */
#include <gssapi.h> /* GSS_C_DELEG_FLAG */
#include <string.h> /* strcmp */

#include "utils.h"



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
            printf("GSS-API ERROR (%s): %s: %s\n", 
                status_type == GSS_C_GSS_CODE ? "GSS-API" : "MECHANISM",    
                msg, 
                (char*)status_message.value
            );
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

/*********************************
*
*   print_oid
*
*   Purpose:
*       Print an OID
*
*   Arguments:
*       oid  - the OID to print
*
*********************************/

void print_oid(gss_OID oid)
{
    unsigned int i = 0;
    char *curr_octet = (char*)oid->elements;

    printf("length: %d\n", oid->length);
    printf("content: ");
    for(i = 0; i < oid->length; ++i)
    {
        printf("0x%x ", (unsigned char)*(curr_octet++));
    }
    printf("\n");
}

/************************************
*
*   print_mechs
*     
*   Purpose:
*       print the number of supported mechanisms
*       by the implementation, and print their OIDs
*
**************************************/

void print_mechs()
{
    OM_uint32 maj_stat, min_stat;
    gss_OID_set mech_set;
    unsigned int i = 0;

    maj_stat = gss_indicate_mechs(&min_stat, &mech_set);
    if(maj_stat != GSS_S_COMPLETE)
    {
        display_status("gss_indicate_mechs", maj_stat, min_stat);
        return;
    }
    printf("number of mechs: %lu\n", mech_set->count);
    
    for(i = 0; i < mech_set->count; ++i)
    {
        printf("mech number: %d\n", i);
        print_oid(mech_set->elements + i);
    }

    gss_release_oid_set(&min_stat, &mech_set);
}


/**************************************************
*
*   test_mech_oid
*
*   Purpose:
*       test if a specific mechanism is supported by the 
*       implementation
*
*   Arguments:
*       mech_oid  - the requested mechanism OID
*
*   Returns:
*       1 for yes, 0 for no, -1 for error
*
**************************************************/

int test_mech_oid(gss_OID mech_oid)
{
    OM_uint32 maj_stat, min_stat;
    gss_OID_set mech_set;
    int present = 0;
    maj_stat = gss_indicate_mechs(&min_stat, &mech_set);
    if(maj_stat != GSS_S_COMPLETE)
    {
        display_status("gss_indicate_mechs", maj_stat, min_stat);
        return -1;
    }
    
    maj_stat = gss_test_oid_set_member(&min_stat, mech_oid, mech_set, &present);
    if(GSS_S_COMPLETE != maj_stat)
    {
        display_status("gss_test_oid_set_member", maj_stat, min_stat);
        return -1;
    }
    gss_release_oid_set(&min_stat, &mech_set);

    return present;
}

/***************************************
*
*   test_mech_str
*
*   Purpose:
*       checks if a mechanism str given by the user
*       has a corresponding OID in our "database"
*
*   Arguments:
*       mech_str - a string describing a mechanism
*
*   Returns:
*       If the mechanism string has been found, returns 
*       the corresponding OID, else, returns NULL
*****************************************/

gss_OID test_mech_str(const char *requested_mech)
{
    unsigned int i = 0;
    for(i = 0; i < sizeof(available_mechs) / sizeof(struct mech_oid); ++i)
    {
        if(!strcmp(available_mechs[i].name, requested_mech))
        {
            return available_mechs[i].oid;
        }
    }
    return NULL;
}