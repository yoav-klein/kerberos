
/***************************************************************

	server.c
	==========
	a kerberized service application that receives kerberos authentication from a client
	
	Usage:
		./server -h <host> -p <port> [-s <service>] [-S keytab]
		
		<host> and <service> must match the service/host of the service principal name in the keytab file

	Example:
		./server -h krb5-server.myexample.com -p 8081 -s host
	
	Flow:
		1. resolve the service principal name with `krb5_sname_to_principal`
		2. open a TCP socket for listening
		3. call `krb5_recvauth` to authenticate the user.


******************************************************************/


#define _POSIX_C_SOURCE 200112

#include <unistd.h> /* getopt */
#include <stdio.h> /* printf */
#include <sys/types.h>   /* socket */
#include <sys/socket.h> /* socket */
#include <stdlib.h> /* atoi */
#include <arpa/inet.h>  /* INADDR_ANY */
#include <string.h> /* memset */
#include <krb5.h> /* krb5_init_context */

#define LISTEN_BACKLOG (50)

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


void bind_socket(int sockfd, int is_specific_addr, char* addr, int port)
{
	struct sockaddr_in servaddr;
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if(is_specific_addr)
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
		exit(1);
	}
	printf("server: Bound to %s\n", inet_ntoa(servaddr.sin_addr));
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
	printf("Waiting for connection...\n");
	cfd = accept(sockfd, (struct sockaddr*)&cliaddr, &client_addr_size);
	if(-1 == cfd)
	{
		perror("accept");
		exit(1);
	}
	
	printf("server: Received connection from: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	
	return cfd;
}


static void usage(const char *name)
{
	printf("Usage: %s [-h host] [-p port] [-s service] [-S keytab] \n", name);
}

void parse_args(int argc, char **argv, int *port, char **service, char **keytab, char **host)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0; 
	
	while((ch = getopt(argc, argv, "p:s:S:h:")) != -1)
	{
		switch(ch)
		{
			case 'p':
				*port = atoi(optarg);
				break;
			case 's':
				*service = optarg;
				break;
			case 'S':
				*keytab = optarg;
				break;
			case 'h':
				*host = optarg;
				break;
			default:
				usage(*argv);
				exit(1);
		}
	}	

}


int main(int argc, char **argv)
{
	int sock = 0, cfd = 0;
	int port = 0;
	char *service = NULL;
	char *keytab_str = NULL;
	char *host = NULL;
	char *client_name = NULL;
	char *service_canonicalized;
	
	krb5_context context;
	krb5_error_code retval;
	krb5_principal server;
	krb5_auth_context auth_context = NULL;
	krb5_keytab keytab = NULL;
	krb5_ticket *ticket;
	
	parse_args(argc, argv, &port, &service, &keytab_str, &host);
	
	if(!port)
	{
		usage(*argv);
		exit(1);
	}
	
	
	/* init context */
	retval = krb5_init_context(&context);
	
	if(keytab_str)
	{
		retval = krb5_kt_resolve(context, keytab_str, &keytab);
		if(retval)
		{
			error_message(retval);
		}
	}
	
	if(retval)
	{
		com_err(argv[0], retval,"krb5_init_context");
		exit(1);
	}	
	
	/* takes a service name and host name as strings, and canonicalize into a principal name */
	retval = krb5_sname_to_principal(context, host, service, KRB5_NT_SRV_HST, &server);
	if(retval)
	{
		com_err(argv[0], retval, "krb5_sname_to_principal");
		exit(1);
	}
	
	
	retval = krb5_unparse_name(context, server, &service_canonicalized);
	printf("server: canonicalized: %s\n", service_canonicalized);
	
	sock = create_socket();
	bind_socket(sock, 0, NULL, port);
	cfd = receive_connection(sock);
	
	/* recvauth */
	retval = krb5_recvauth(context, &auth_context, (krb5_pointer)&cfd, 
			"version5", server, 
			0 /* no flags*/, 
			keytab /* NULL is default keytab*/,
			&ticket);

	if(retval)
	{
		com_err(argv[0], retval, "after recvauth");
		exit(1);
	}
	
	
	retval = krb5_unparse_name(context, ticket->enc_part2->client, &client_name);
	if(retval)
	{
		error_message(retval);
	}
	
	printf("server: Authenticated !!!\n");
	printf("server: You are: %s\n", client_name);
	
	/*  freeing structures */
	krb5_free_ticket(context, ticket);
	krb5_free_principal(context, server);
	if(keytab)
	{
		krb5_kt_close(context, keytab);
	}
	krb5_auth_con_free(context, auth_context);
	krb5_free_context(context);
	
	return 0;
}




