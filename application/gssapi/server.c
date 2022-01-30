#include <stdio.h> /* printf */
#include <getopt.h> /* getopt */
#include <stdlib.h> /* exit */

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

int main(int argc, char **argv)
{
    int port = 0;
    char *service = NULL;
    char *mech = NULL;
    parse_args(argc, argv, &port, &mech, &service);

    printf("port: %d, service: %s\n", port, service);
    return 0;
}