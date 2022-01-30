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


/*

NOTES

1. Didn't use OID stuff here
2. How is the service name received? what format?



*/


void usage()
{
	printf("client [-p <port>] [-d] [-m <mech>] host service [-f] message");
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
	gss_buffer_desc gss_name, send_token, recv_token, *recv_token_ptr;
	gss_name_t target_name;
	OM_uint32 maj_stat, min_stat;
	
	/* import the service name into target_name */
	gss_name.value = service;
	gss_name.length = strlen(gss_name.value) + 1;
	
	maj_stat = gss_import_name(&min_stat, &gss_name, 
     (gss_OID)GSS_C_NT_HOSTBASED_SERVICE, &target_name);
     
     if(maj_stat != GSS_S_COMPLETE)
     {
     	printf("gss_import_name: Something is wrong\n");
     	/* TODO: gss_display_status */
     	exit(1);
     }
     
     recv_token_ptr = GSS_C_NO_BUFFER;
     *gss_context = GSS_C_NO_CONTEXT;
     
     do
     {
		maj_stat = gss_init_sec_context(&min_stat,
			GSS_C_NO_CREDENTIAL, /* indicate default credentials */
			target_name, /* name of the server */
			mech_type,
			GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
			0, /* no time req */
			NULL, /* no channel binding */
			recv_token_ptr, /* this is what we get from peer, first it's null */
			NULL, /* ignore mech type */
			&send_token, /* this will be sent to the peer */
			ret_flags,
			NULL /* ignore time rec */
     }		
     while()
     
}

int call_server(char *host, char *port, char *service, char *message)
{
	int sock = 0;
	
	sock = connect_to_server(host, service);
	if(-1 == sock)
	{
		printf("failed to connect to server\n");
		exit(1);
	}
	
	

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
	
	call_server(host, port, service, "Give me your file\n");
	
	
	return 0;
}



