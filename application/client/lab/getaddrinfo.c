/*******************************************
*	
*	This demonstrate the use of getaddrinfo and getnameinfo
*
*
**********************************************/

#define _POSIX_C_SOURCE 200112

#define NI_MAXHOST 1025
#define NI_MAXSERV 32

#include <stdio.h> /* printf */
#include <stdlib.h> /* exit */
#include <netdb.h> /* struct addrinfo */
#include <string.h> /* memset */
#include <arpa/inet.h> /* inet_ntoa */


int main(int argc, char **argv)
{
	char *service = NULL;
	char *node = NULL;
	char host_buff[NI_MAXHOST];
	char serv_buff[NI_MAXSERV];
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int status = 0;
	int count = 0;
	
	if(argc < 3)
	{
		printf("Usage: %s <host> <port>\n", argv[0]);
		exit(1);
	}
	
	node = argv[1];
	service = argv[2];
	
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_INET; /* IPv4 or IPv6 */
	hints.ai_socktype = 0; /* TCP or UDP */
	hints.ai_protocol = 0; /* any protocol */
	hints.ai_flags = 0; /* no flags */
	
	status = getaddrinfo(node, service, &hints, &result);
	
	for(rp = result; rp != NULL; rp = rp->ai_next)
	{
		struct sockaddr_in *inet_addr = { 0 };
		printf("Found address %d:\n", count++);
		inet_addr = (struct sockaddr_in*)rp->ai_addr;
		printf("IP: %s, Port: %d, Family: %d, Type: %d\n", inet_ntoa(inet_addr->sin_addr), ntohs(inet_addr->sin_port), rp->ai_family, rp->ai_socktype);
		
		/* now translate back from addrinfo to name */
		getnameinfo(rp->ai_addr, rp->ai_addrlen, host_buff, sizeof(host_buff), serv_buff, sizeof(serv_buff), 0);
		printf("Host: %s\n", host_buff);
		printf("Service: %s\n", serv_buff);
		printf("-----------\n");
	}

	return 0;
}
