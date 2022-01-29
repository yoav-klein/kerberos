
/*******************************************************************

	client.c
	==========
	a client application that communicates with a kerberized server application
	using the libkrb5 API.
	
	Usage:
		./client -h <server_host> -p <port> [-s <service>]
	
	Example:
		./client -h krb5-server.myexample.com -p 8081 -s host
	
	Flow:
		1. using the host, the IP of the server is retrieved (using DNS).
		2. using the host and the optional service, a service principal name is composed
		using `krb5_sname_to_principal`.
		3. the default credentials cache is retrieved and stored in ccdef
		4. the default principal is retrieved from the credentials cache (assuming kinit was performed)
		5. the krb5_sendauth function is called, which sends a AP_REQ to the server.
		
	NOTES:
	* krb5_sname_to_principal
		This function performs a forward DNS lookup to find the IP address of the domain name, and then
		a reverse lookup (PTR) to find back the domain name, and by this composes the principal name.
		That can be problematic since the reverse lookup can yield a different principal name than the one configured in the KDC
		So we disabled this by setting `rnds = false` in the krb5.conf file.
	* krb_cc_default
		This function gets the default credentials cache name and stores it in a `krb5_ccache` struct
	* krb5_cc_get_principal
		This function gets the default principal from the credentials cache.
	* krb5_sendauth
		This function will look for a ticket for the particular service in the credentials cache, and if not found,
		will issue a TGS_REQ message to the TGS, asking for a ticket. It will then send the ticket to the server.





*********************************************************************/




#define _POSIX_C_SOURCE 200112

#include <stdio.h> /* pritf */
#include <krb5.h> /* krb5_init_context */
#include <stdlib.h> /* exit */
#include <netdb.h>  
#include <sys/types.h> /* struct addrinfo */
#include <string.h> /* memset */
#include <unistd.h> /* getopt */
#include <sys/socket.h> /* socket */
#include <netinet/in.h> /* inet_ntoa */
#include <arpa/inet.h> /*  inet_ntoa */


static void usage(const char *name)
{
    printf("Usage: %s -h <host> -p <port> [-s <service>]\n", name);

}

void parse_args(int argc, char **argv, char **host, char **port, char **service)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0;
	
	while((ch = getopt(argc, argv, "h:p:s:")) != -1)
	{
		switch(ch)
		{
			case 'p':
				*port = optarg;
				break;
			case 's':
				*service = optarg;
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

void print_found_address(struct addrinfo *entry)
{
	struct sockaddr_in *inet_addr = { 0 };
	inet_addr = (struct sockaddr_in*)entry->ai_addr;
	printf("IP: %s, Port: %d, Family: %d, Type: %d\n", inet_ntoa(inet_addr->sin_addr), 
	    ntohs(inet_addr->sin_port), entry->ai_family, entry->ai_socktype);
}

void printf_found_addresses(struct addrinfo *result)
{
    int count = 0;
    struct addrinfo *runner = result;
    for(runner = result; runner != NULL; runner = runner->ai_next)
	{
    	printf("Found address %d:\n", count++);
	    print_found_address(runner);	
	}
}

int main(int argc, char **argv)
{
    int status = 0;
    int sock = 0;
    
    krb5_error_code retval;
	krb5_context context;
	krb5_principal server, client;
	krb5_ccache ccdef;
	krb5_auth_context auth_context = 0;
	krb5_error *err_ret;
    krb5_ap_rep_enc_part *rep_ret;
    krb5_data data;
	
	struct addrinfo aihints, *res_addresses, *runner;
	char *host = NULL;
	char *service = NULL;
	char *port = NULL;
	char *service_principal_name = NULL;
	char *client_principal_name = NULL;
	
	parse_args(argc, argv, &host, &port, &service);
	if(argc < 2)
	{
	    usage(argv[0]);
	    exit(1);
	}
	
	retval = krb5_init_context(&context);
	if(retval)
	{
		error_message(retval);
		exit(1);
	}
	
    memset(&aihints, 0, sizeof(aihints));
    aihints.ai_family = AF_INET;
    aihints.ai_socktype = SOCK_STREAM;
	
	status = getaddrinfo(host, port, &aihints, &res_addresses);
	if(status)
	{
	    printf("%s\n", gai_strerror(status));
	    exit(1);
	}
	/*printf_found_addresses(res_addresses);*/ /* for debugging and diagnostics */
	
	retval = krb5_sname_to_principal(context, host, service, KRB5_NT_SRV_HST, &server);
	
	/* print canonicalized */
	retval = krb5_unparse_name(context, server, &service_principal_name);
	printf("client: canonicalized service name: %s\n", service_principal_name);
		
	for(runner = res_addresses; runner != NULL; runner = runner->ai_next)
	{
	    printf("Trying to connect to:\n");
	    print_found_address(runner);
	    sock = socket(runner->ai_family, runner->ai_socktype, 0);
	    if(-1 == sock)
	    {
	        perror("socket failed");
	        exit(1);
	    }
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
	
	/* resolve the default credential cache name */
	retval = krb5_cc_default(context, &ccdef);
	if(retval)
	{
	    com_err(argv[0], retval, "while getting default ccache");
	    exit(1);
	}
		
	/* get the default principal of a credential cache */
	retval = krb5_cc_get_principal(context, ccdef, &client);
	if(retval)
	{
	    com_err("krb5_cc_get_principal", retval, "while getting client principal name");
	    exit(1);
	}
	
	/* display the retrieved default principal from credential cache */
	retval = krb5_unparse_name(context, client, &client_principal_name);
	if(retval)
	{
		com_err("krb5_unparse_name", retval, "while parsing client principal name");
		exit(1);
	}
	printf("client: client principal name: %s\n", client_principal_name);
	
	data.data = "Kukuriku";
	data.length = strlen(data.data);
	
	/*
	krb5_sendauth:
		- retrieves credentials for client to server. First trying from 
		  credentials cache, then sends a TGS_REQ to the Ticket Granting Server
	
	*/
	retval = krb5_sendauth(context, &auth_context, (krb5_pointer)&sock, 
	                        "version5", client, server, AP_OPTS_MUTUAL_REQUIRED, &data,
	                        0, /* no creds, use credential cache */ ccdef, &err_ret, &rep_ret, NULL);
	
	
	if(retval)
    {
        com_err(argv[0], retval, "after sentauth");
        error_message(retval);
        exit(1);
    }
	
	printf("client: Authenticated successfully!\n");
	krb5_free_unparsed_name(context, client_principal_name);
	krb5_free_unparsed_name(context, service_principal_name);
	krb5_cc_close(context, ccdef);
	krb5_auth_con_free(context, auth_context);
	krb5_free_principal(context, client);
	krb5_free_principal(context, server);
	freeaddrinfo(res_addresses);
	krb5_free_context(context);
	
	return 0;
}
