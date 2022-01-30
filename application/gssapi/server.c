
#include <stdio.h> /* printf */
#include <getopt.h> /* getopt */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <gssapi.h> /* gss_cred_id_t */

#include "utils.h"

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

/**********************************
*   acquire_creds
*
*   acquiring credentials for the server and 
*   stores them in out_creds
*
************************************/

int acquire_creds(char *service_name, gss_cred_id_t *out_creds)
{
    gss_buffer_desc name_buffer;
    gss_buffer_desc name_buffer2;
    gss_name_t server_name;
    OM_uint32 maj_stat, min_stat;

    name_buffer.value = service_name;
    name_buffer.length = strlen(name_buffer.value) + 1;

    maj_stat = gss_import_name(&min_stat, &name_buffer, 
                               (gss_OID)GSS_C_NT_HOSTBASED_SERVICE, &server_name);
    
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("server import name failed\n");
        return -1;
    }

    printf("name of service:\n");
    maj_stat = gss_display_name(&min_stat, server_name, &name_buffer2, NULL);
    printf("%s\n", (char*)name_buffer2.value);

    maj_stat = gss_acquire_cred(&min_stat, server_name,
                                0, /* no time req */
                                GSS_C_NULL_OID_SET, /* desired mechanisms, indicate default */
                                GSS_C_ACCEPT, /* cred usage*/
                                out_creds,
                                NULL, NULL
                                );
    gss_release_name(&min_stat, &server_name);
    if(GSS_S_COMPLETE != maj_stat)
    {
        printf("gss_acquire_cred failed\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int port = 0;
    char *service = NULL;
    char *mech = NULL;

    gss_cred_id_t server_creds;

    parse_args(argc, argv, &port, &mech, &service);

    printf("port: %d, service: %s\n", port, service);

    if(-1 == acquire_creds(service, &server_creds))
    {
        printf("couldn't acquire creds\n");
        exit(1);
    }

    return 0;

}