
/*****************************************
*   
*   server.c
*
*   a server application that authenticates the client
*   using GSS-API
*
*   Usage:
*       $ server [-p <port>] [-m <mech>] service
*
*   Flow:
*       1. acquires the service credentials
*       2. 
*
********************************************/


#include <stdio.h> /* printf */
#include <getopt.h> /* getopt */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <gssapi.h> /* gss_cred_id_t */
#include <sys/types.h>   /* socket */
#include <sys/socket.h> /* socket */
#include <arpa/inet.h>  /* INADDR_ANY */

#include "utils.h"

#define LISTEN_BACKLOG (50)

void usage()
{
	printf("server [-p <port>] [-m <mech>] service\n");
}

void parse_args(int argc, char **argv, int *port, char **mech, char **service)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0;
		
	while(-1 != (ch = getopt(argc, argv, "p:m:")))
	{
		switch(ch)
		{
			case 'p':
				*port = atoi(optarg);
				break;
			case 'm':
				*mech = optarg;
				break;
			case '?':
				if(optopt == 'p')
				{
					printf("option p requires an argumet\n");
				}
                if(optopt == 'm')
                {
                    printf("option m requires an argument\n");
                }
				break;
		}
	}
	
	if(argc - optind < 1)
	{
		usage();
		exit(1);
	}
	
	*service = argv[optind++];
}



int create_socket()
{
	int opt = 1; 
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket creation");
		exit(1);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
												&opt, sizeof(opt)))
	{
		perror("setsockopt"); 
		exit(EXIT_FAILURE);
	}
	
	return sockfd;
}


int bind_socket(int sockfd, char* addr, int port)
{
	struct sockaddr_in servaddr;
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if(addr)
	{
		servaddr.sin_addr.s_addr = inet_addr(addr);
	}
	else
	{
		servaddr.sin_addr.s_addr = INADDR_ANY;
	}
	servaddr.sin_port = htons(port);
	
	if(-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
	{
		perror("bind");
		return -1;
	}
	printf("server: Bound to %s\n", inet_ntoa(servaddr.sin_addr));

    return 0;
}



int receive_connection(int sockfd)
{
	int cfd = 0;
	struct sockaddr_in cliaddr;
	socklen_t client_addr_size;	
		
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	if(-1 == listen(sockfd, LISTEN_BACKLOG))
	{
		perror("listen");
		exit(1);
	}
	
	client_addr_size = sizeof(cliaddr);
	printf("server: Waiting for connection...\n");
	cfd = accept(sockfd, (struct sockaddr*)&cliaddr, &client_addr_size);
	if(-1 == cfd)
	{
		perror("accept");
		exit(1);
	}
	
	printf("server: Received connection from: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	
	return cfd;
}


/**********************************
*
*   acquire_creds
*
*   acquiring credentials for the server and 
*   stores them in out_creds
*
************************************/

int acquire_creds(char *service_name, gss_cred_id_t *out_creds)
{
    gss_buffer_desc gssbuff_service_name;
    gss_buffer_desc gssbuff_service_name2;
    gss_name_t gssname_service_name;
    OM_uint32 maj_stat, min_stat;

    gssbuff_service_name.value = service_name;
    gssbuff_service_name.length = strlen(gssbuff_service_name.value) + 1;

    maj_stat = gss_import_name(&min_stat, &gssbuff_service_name, 
                               (gss_OID)GSS_C_NT_HOSTBASED_SERVICE, &gssname_service_name);
    
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("server import name failed\n");
        return -1;
    }

    printf("name of service:\n");
    maj_stat = gss_display_name(&min_stat, gssname_service_name, &gssbuff_service_name2, NULL);
    printf("%s\n", (char*)gssbuff_service_name2.value);

    maj_stat = gss_acquire_cred(&min_stat, gssname_service_name,
                                0, /* no time req */
                                GSS_C_NULL_OID_SET, /* desired mechanisms, indicate default */
                                GSS_C_ACCEPT, /* cred usage*/
                                out_creds,
                                NULL, NULL
                                );
    gss_release_name(&min_stat, &gssname_service_name);
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("server: gss_acquire_cred failed\n");
        return -1;
    }
    return 0;
}

/********************************
*   
*   establish_context
*
*   establishes a GSSAPI context with the client
*   returns a context handle and a client name
*
*********************************/
int establish_context(int fd, gss_cred_id_t server_creds, 
    gss_ctx_id_t *context,
    gss_buffer_desc *gssbuf_client_name,
    OM_uint32 *ret_flags)
{
    gss_OID mech_oid;
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc recv_tok, send_tok;
    gss_name_t gssname_client_name;
    do
    {   
        if(-1 == recv_token(fd, &recv_tok))
        {
            printf("server: establish_context: couldn't receive token");
            return -1;
        }

        *context = GSS_C_NO_CONTEXT;
         
        maj_stat = gss_accept_sec_context(&min_stat,
            context, /* first we supploy GSS_C_NO_CONTEXT */
            server_creds,
            &recv_tok, /* token received from client */
            GSS_C_NO_CHANNEL_BINDINGS,
            &gssname_client_name, /* gets the name of the initiator, must be freed with gss_release_name */
            &mech_oid,
            &send_tok, /* token to send to the peer */
            ret_flags,
            NULL, /* ignore time rec */
            NULL /* ignore delegate_cred_handle */
        );
        
        if(GSS_C_NO_BUFFER != &recv_tok)
        {
            gss_release_buffer(&min_stat, &recv_tok);
        }

        if(GSS_S_COMPLETE != maj_stat && GSS_S_CONTINUE_NEEDED != maj_stat)
        {
            printf("server: gss_accept_sec_context failed!\n");
            return -1;
        }
        if(GSS_S_CONTINUE_NEEDED)
        {
            printf("server: establish_context: continue needed..\n");
        }
        
        if(send_tok.length > 0)
        {
            printf("server: establish_context: sending token to client\n");
            if(-1 == send_token(fd, &send_tok))
            {
                printf("server: establish_context: couldn't send token to client\n");
                gss_release_buffer(&min_stat, &send_tok);
                gss_delete_sec_context(&min_stat, context, NULL);
            }
            gss_release_buffer(&min_stat, &send_tok);
        }
        
    }
    while(GSS_S_COMPLETE != maj_stat);

    /* TODO: display context flags */


    return 0;
}


/********************************
*   
*   sign_server 
*   
*   establishes a context with the client
*   signs the received message with gss_get_mic
*   and sends it back to the client
*
*********************************/

int sign_server(int fd, gss_cred_id_t server_creds)
{
    gss_ctx_id_t context;
    gss_buffer_desc gssbuff_client_name;
    gss_buffer_desc recv_tok;
    gss_buffer_desc message;
    OM_uint32 maj_stat, min_stat;
    OM_uint32 ret_flags = 0;
    int conf_state = 0;

    if(-1 == establish_context(fd, server_creds, &context,
        &gssbuff_client_name, &ret_flags))
    {
        printf("server: couldn't establish context\n");
        return -1;
    }

    printf("server: context established with: %s\n", (char*)gssbuff_client_name.value);

    gss_release_buffer(&min_stat, &gssbuff_client_name);

    if(-1 == recv_token(fd, &recv_tok))
    {
        printf("server: couldn't receive token from client");
        gss_delete_sec_context(&min_stat ,&context, NULL);
        return -1;
    }

    /* TODO: print token*/

    maj_stat = gss_unwrap(&min_stat, context, 
        &recv_tok, 
        &message, /*  message unwrapped. must be released with gss_release_buffer */
        &conf_state, /* 0 - integrity only. 1 - integrity & confidentiality */
        NULL /* don't care about QOP */
    );
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("server: gss_unwrap failed !\n");
        gss_delete_sec_context(&min_stat ,&context, NULL);
    }
    if(!conf_state)
    {
        printf("Warning ! message not encrypted!\n");
    }

    gss_release_buffer(&min_stat, &recv_tok);

    /* display recevied message */
    printf("server: received message:\n");
    printf("%s\n", (char*)message.value);

    /* produce signature for the message */
    maj_stat = gss_get_mic(&min_stat, context,
        GSS_C_QOP_DEFAULT, /* request default QOP */
        &message, /* input message */
        &recv_tok /* buffer to store the token it. must be freed later with gss_release_buffer */
    );
    gss_release_buffer(&min_stat, &message); /* release the message buffer anyway */
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("server: gss_get_mic failed\n");
        gss_delete_sec_context(&min_stat ,&context, NULL);
    }

    if(-1 == send_token(fd, &recv_tok))
    {
        printf("server: couldn't send verification message to client\n");
        gss_release_buffer(&min_stat, &recv_tok);
        gss_delete_sec_context(&min_stat, &context, NULL);
    }
    gss_release_buffer(&min_stat, &recv_tok);
    
    gss_delete_sec_context(&min_stat, &context, NULL);

    return 0;

}

int talk_to_client(int port, char *service, char *mech)
{
    gss_cred_id_t server_creds;

    int sock = 0;
    int cfd = 0;

    if(-1 == acquire_creds(service, &server_creds))
    {
        printf("server: couldn't acquire creds\n");
        exit(1);
    }

    sock = create_socket();
    if(-1 == bind_socket(sock, NULL, port))
    {
        printf("server: couldn't bind socket\n");
        return -1;
    }

    cfd = receive_connection(sock);
    if(-1 == sign_server(cfd, server_creds))
    {
        printf("server: talk_to_client failed\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int port = 0;
    
    char *service = NULL;
    char *mech = NULL;

    parse_args(argc, argv, &port, &mech, &service);

    printf("port: %d, service: %s\n", port, service);

    if(-1 == talk_to_client(port, service, mech))
    {
        return -1;
    }
    


    return 0;

}