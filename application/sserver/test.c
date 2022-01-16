#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void parse_args(int argc, char **argv, int *port, char **service)
{
	extern int opterr, optind;
	extern char *optarg;
	char ch = 0;
	int i = 0;
	while((ch = getopt(argc, argv, "p:s:")) != -1)
	{
		switch(ch)
		{
			case 'p':
				*port = atoi(optarg);
				break;
			case 's':
				*service = optarg;
				break;
			default:
				printf("Wrong usage\n");
				exit(1);
		}
	}	

}

int main(int argc, char **argv)
{
	
	int port = 0;
	char *service;
	
	parse_args(argc, argv, &port, &service);
	
	printf("port: %d\n", port);
	printf("service: %s\n", service);

	return 0;
}
