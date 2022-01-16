#define _POSIX_C_SOURCE 2

#include <unistd.h> /* getopt */
#include <stdio.h> /* printf */
#include <sys/types.h>   /* socket */
#include <sys/socket.h> /* socket */
#include <stdlib.h> /* atoi */
#include <arpa/inet.h>  /* INADDR_ANY */
#include <string.h> /* memset */
#include <krb5.h> /* krb5_init_context */


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
	printf("Bound to %s\n", inet_ntoa(servaddr.sin_addr));
}


static void usage(const char *name)
{
	printf("Usage: %s [-p port] [-s service] [-S keytab] [-h host]\n", name);
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
	int port = 0;
	char *service = NULL;
	char *keytab = NULL;
	char *host = NULL;
	char *service_canonicalized;
	
	krb5_context context;
	krb5_error_code retval;
	krb5_principal server;
	
	parse_args(argc, argv, &port, &service, &keytab, &host);
	
	printf("port: %d\n", port);
	printf("service: %s\n", service);
	printf("host: %s\n", host);
	printf("keytab: %s\n", keytab);
	
	/* init context */
	retval = krb5_init_context(&context);
	if(retval)
	{
		perror("krb5_init_context");
		exit(1);
	}	
	
	/* takes a service name and host name as strings, and canonicalize into a principal name */
	retval = krb5_sname_to_principal(context, host, service, KRB5_NT_SRV_HST, &server);
	if(retval)
	{
		perror("krb5_sname_to_principal");
		exit(1);
	}
	retval = krb5_unparse_name(context, server, &service_canonicalized);
	printf("%s\n", service_canonicalized);
	
	return 0;
}




