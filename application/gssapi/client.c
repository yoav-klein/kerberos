
#include <stdlib.h> /* atoi */
#include <getopt.h> /* getopt */
#include <stdio.h> /* printf */

void usage()
{
	printf("client [-p <port>] [-d] [-m <mech>] host service [-f] message");
}

void parse_args(int argc, char **argv, int *port, int *delegate, char **mech, char **host, char **service, char **msg)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0;
	int i = 0;
		
	while(-1 != (ch = getopt(argc, argv, "p:m:d")))
	{
		switch(ch)
		{
			case 'p':
				*port = atoi(optarg);
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


int main(int argc, char **argv)
{
	int port = 8080;
	int delegate = 0;
	char *mech = NULL;
	char *file = NULL;
	char *host = NULL;
	char *service = NULL;
	char *message = NULL;
	parse_args(argc, argv, &port, &delegate, &mech, &host, &service, &message);
	printf("port: %d, delegate: %d, mech: %s, file: %s\n", port, delegate, mech, file);
	printf("host: %s, service: %s, message: %s\n", host, service, message);
	
	
	return 0;
}
