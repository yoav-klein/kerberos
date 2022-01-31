
/*****************************************************
*	
*	client.c
*	
*	communicates with the server using GSS-API
*
*	Usage:
*		$ client [-p <port>] [-d] [-m <mech>] host service [-f] message
*
*		
*	Flow:
*		1. connecting to the server using TCP
*		2. imports the service name to GSS-API internal structure
*		3. creates a security context with the server
*		4. wrap a message with gss_wrap
*		5. send the message to peer
*		6. read a MIC back from the peer and verify it, for "transmission confirmation"
*
***********************************************************/



#define _POSIX_C_SOURCE 200112


#include <stdlib.h> /* atoi */
#include <getopt.h> /* getopt */
#include <stdio.h> /* printf */
#include <sys/socket.h> /* socket */
#include <sys/types.h> /* getaddrinfo */
#include <netdb.h> /* getaddrinfo */
#include <netinet/in.h> /* inet_ntoa */
#include <arpa/inet.h> /*  inet_ntoa */
#include <gssapi.h> /* gss_context */
#include <string.h> /* strlen */
#include <unistd.h> /* close */

#include "utils.h"

/*

NOTES

1. Didn't use OID stuff here
2. How is the service name received? what format?s

*/


void usage()
{
	printf("client [-p <port>] [-d] [-m <mech>] host service [-f] message\n");
}

void parse_args(int argc, char **argv, char **port, int *delegate, char **mech, char **host, char **service, char **msg)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0;
		
	while(-1 != (ch = getopt(argc, argv, "p:m:d")))
	{
		switch(ch)
		{
			case 'p':
				*port = optarg;
				break;
			case 'd':
				*delegate = 1;
				break;
			case 'm':
				*mech = optarg;
				break;
			case '?':
				if(optopt == 'p')
				{
					printf("option p requires an argumet\n");
				}
				break;
		}
	}
	
	if(argc - optind < 2)
	{
		usage();
		exit(1);
	}
	
	*host = argv[optind++];
	*service = argv[optind++];
	*msg = argv[optind++];
}

void print_found_address(struct addrinfo *entry)
{
	struct sockaddr_in *inet_addr = { 0 };
	inet_addr = (struct sockaddr_in*)entry->ai_addr;
	printf("IP: %s, Port: %d, Family: %d, Type: %d\n", inet_ntoa(inet_addr->sin_addr), 
	    ntohs(inet_addr->sin_port), entry->ai_family, entry->ai_socktype);
}


int connect_to_server(char *host, char *port)
{
	int sock = 0;
	int status = 0;
	struct addrinfo aihints = { 0 }, *res_addresses, *runner;
	
	aihints.ai_family = AF_INET;
	aihints.ai_socktype = SOCK_STREAM;
	
	status = getaddrinfo(host, port, &aihints, &res_addresses);
	if(status)
	{
		printf("%s\n", gai_strerror(status));
		return -1;
	}
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sock)
	{
		perror("socket\n");
		return -1;
	}
	
	
	for(runner = res_addresses; runner != NULL; runner = runner->ai_next)
	{
	    printf("Trying to connect to:\n");
	    print_found_address(runner);
	    if(connect(sock, runner->ai_addr, runner->ai_addrlen))
	    {   
	        perror("connect failed, trying the next\n");
	        continue;
	    }
	    else
	    {
	        printf("connected !\n");
	        break;
	    }
	}
	
	return sock;
	
}

/***************
*	establish_context
*
*	Establishes a GSS-API context  with a specified service and returns 
*	a context handle
*
*	Arguments:
*	fd           -  file descriptor of a TCP socket of an opened connection with the server
*	service_name - the name of the service
*	mech_type    - mechanism type. should be GSS_C_NULL_OID
*	ctx          - output parameter - returned context
*	ret_flags    - returned flags from init_sec_context
*
*	returns -1 on error
*
***************/


int establish_context(int fd, char *service_name, gss_ctx_id_t *ctx, 
				const gss_OID mech_type, OM_uint32 *ret_flags)
{
	gss_buffer_desc gss_name, send_tok, recv_tok, *recv_tok_ptr;
	gss_name_t target_name;
	OM_uint32 maj_stat, min_stat, lifetime;
	
	
	/* import the service name into target_name */
	gss_name.value = service_name;
	gss_name.length = strlen(gss_name.value) + 1;
	
	maj_stat = gss_import_name(&min_stat, &gss_name, 
     (gss_OID)GSS_C_NT_HOSTBASED_SERVICE, &target_name);
     
	if(maj_stat != GSS_S_COMPLETE)
	{
	printf("gss_import_name: Something is wrong\n");
	/* TODO: gss_display_status */
	return -1;
	}
	
	recv_tok_ptr = GSS_C_NO_BUFFER;
	*ctx = GSS_C_NO_CONTEXT;
	
	do
	{
		maj_stat = gss_init_sec_context(&min_stat,
			GSS_C_NO_CREDENTIAL, /* indicate default credentials */
			ctx,
			target_name, /* name of the server */
			mech_type,
			GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
			0, /* no time req */
			NULL, /* no channel binding */
			recv_tok_ptr, /* this is what we get from peer, first it's null */
			NULL, /* ignore mech type */
			&send_tok, /* this will be sent to the peer */
			ret_flags,
			NULL /* ignore time rec */
			);
		if(NULL == ctx)
		{
			printf("couldn't create context\n");
			return -1;
		}
		/* if there's something in the recv_token */
		if(GSS_C_NO_BUFFER != recv_tok_ptr)
		{
			gss_release_buffer(&min_stat, recv_tok_ptr);
		}
		/* if not one of these, an error occured */
		if((GSS_S_COMPLETE != maj_stat) && (GSS_S_CONTINUE_NEEDED != maj_stat))
		{
			printf("initializing context failed\n");
			gss_release_name(&min_stat, &target_name);
			return -1;
		}
		if(send_tok.length > 0)
		{
			printf("Sending token to server\n");
			if(-1 == send_token(fd, &send_tok))
			{
				gss_release_name(&min_stat, &target_name);
				gss_release_buffer(&min_stat, &send_tok);
				return -1;
			}
			gss_release_buffer(&min_stat, &send_tok);
			
			if(GSS_S_CONTINUE_NEEDED == maj_stat)
			{
				printf("Continue needed...\n");
				if(-1 == recv_token(fd, &recv_tok))
				{
					gss_release_name(&min_stat, &target_name);
					return -1;
				}
				recv_tok_ptr = &recv_tok;
			}
		}
    }
    while(GSS_S_CONTINUE_NEEDED == maj_stat);

    return 0;
}


/*
	questions

	1. context_flags vs ret_flags? are the same?
	2. 

*/
int call_server(char *host, char *port, char *service, char *message)
{
	int res = 0;
	int sock = 0;
	int is_open = 0;
	int conf_req_flag = 0;
	int conf_state = 0;

	gss_ctx_id_t context;
	gss_name_t src_name, target_name;
	gss_buffer_desc sname, tname;
	gss_buffer_desc in_buff, out_buffer;
	OM_uint32 req_output_size = 1012; /* not sure why, this is TCP.. */
	OM_uint32 ret_flags, lifetime, context_flags;
	OM_uint32 maj_stat, min_stat;
	OM_uint32 max_input_size = 0;
	gss_OID mechanism, name_type;

	sock = connect_to_server(host, service);
	if(-1 == sock)
	{
		printf("failed to connect to server\n");
		exit(1);
	}
	
	res = establish_context(sock, service, &context, GSS_C_NULL_OID, &ret_flags);
	if(-1 == res)
	{
		printf("context establishment failed\n");
		exit(1);
	}
	
	/* get context information */
	maj_stat = gss_inquire_context(&min_stat, context, 
		&src_name, /* name of initiator. must be freed with free_name */
		&target_name, /* name of the acceptor. must be freed with free_name */
		&lifetime, /* lifetime of context */
		&mechanism, /* mechanism of the context. pointer to static storage - don't attempt to free */
		&context_flags, /* flags.  */
		NULL, /* whether or not this application is the initiator. not needed */
		&is_open /* whether or not the context is fully established */
	);
	if(GSS_S_NO_CONTEXT == maj_stat)
	{
		printf("context could not be accessed\n");
		return -1;
	}
	if(!(GSS_S_COMPLETE == maj_stat))
	{
		printf("gss_inquire_contetx failed\n");
		return -1;
	}

	/* get wrap size limit without confidentiality */
	maj_stat = gss_wrap_size_limit(&min_stat, context,
		0, /* NOT required confidentiality */
		GSS_C_QOP_DEFAULT, /* default QOP of underlying mechanism */
		req_output_size,
		&max_input_size
		);
	
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_wrap_size_limit failed\n");
		return -1;
	}

	printf("max input size without confidentiality: %d\n", max_input_size);

	/* get wrap size limit with confidentiality */
	maj_stat = gss_wrap_size_limit(&min_stat, context,
		1, /* required confidentiality */
		GSS_C_QOP_DEFAULT, /* default QOP of underlying mechanism */
		req_output_size,
		&max_input_size
		);
	
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_wrap_size_limit failed\n");
		return -1;
	}

	printf("max input size with confidentiality: %d\n", max_input_size);

	/* get names of server and client */
	maj_stat = gss_display_name(&min_stat, src_name, &sname, &name_type);
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_display_name failed\n");
		return -1;
	}

	maj_stat = gss_display_name(&min_stat, target_name, &tname, &name_type);
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_display_name failed\n");
		return -1;
	}

	printf("%s to %s, lifetime: %u, flags: %x, %s\n",
			(char*)sname.value,
			(char*)tname.value,
			lifetime,
			context_flags,
			(is_open)? "open": "close");
	
	/* release resources */
	gss_release_name(&min_stat, &src_name);
	gss_release_name(&min_stat, &target_name);
	gss_release_buffer(&min_stat, &sname);
	gss_release_buffer(&min_stat, &tname);

	/* find out if confidentiality is supported */
	if(ret_flags & GSS_C_CONF_FLAG)
	{
		conf_req_flag = 1;
	}
	else
	{
		conf_req_flag = 0;
	}

	/* preparing the message */
	in_buff.value = message;
	in_buff.length = strlen(message) + 1;

	/* TODO: Skipping some stuff here, see the reference */
	maj_stat = gss_wrap(&min_stat,
		context,
		conf_req_flag, /* 1 - both integrity and confidentiality are requested, 0 - only integrity */
		GSS_C_QOP_DEFAULT,
		&in_buff,
		&conf_state, /* output parameter indicating whether or not confidentiality applied */
		&out_buffer /* output to send to the peer */
		);
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_wrap failed\n");
		return -1;
	}

	if(!conf_state)
	{
		printf("Warning! No confidentiality provided !\n");
	}

	/* send to the server */
	if(-1 == send_token(sock, &out_buffer))
	{
		printf("couldn't send message to peer\n");
		return -1;
	}
	gss_release_buffer(&min_stat, &out_buffer);
	
	/* read verification from the server */
	if(-1 == recv_token(sock, &out_buffer))
	{
		printf("couldn't receive verification message from peer\n");
		return -1;
	}

	/* verify MIC */	
	maj_stat = gss_verify_mic(&min_stat, context,
		&in_buff,
		&out_buffer,
		NULL /* QOP of MIC - not interested */
		);
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_verify_mic failed\n");
		return -1;
	}

	gss_release_buffer(&min_stat, &out_buffer);

	maj_stat = gss_delete_sec_context(&min_stat, &context, GSS_C_NO_BUFFER);
	if(GSS_S_COMPLETE != maj_stat)
	{
		printf("gss_delete_sec_context failed\n");
		return -1;
	}

	close(sock);

	return 0;
}

int main(int argc, char **argv)
{
	char *port = "8080";
	int delegate = 0;
	char *mech = NULL;
	char *file = NULL;
	char *host = NULL;
	char *service = NULL;
	char *message = NULL;
	parse_args(argc, argv, &port, &delegate, &mech, &host, &service, &message);
	printf("port: %s, delegate: %d, mech: %s, file: %s\n", port, delegate, mech, file);
	printf("host: %s, service: %s, message: %s\n", host, service, message);
	
	if(-1 == call_server(host, port, service, "Give me your file\n"))
	{
		printf("call server failed !\n");

		return 1;
	}
	
	return 0;
}

