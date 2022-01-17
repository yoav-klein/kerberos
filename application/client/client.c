

#include <krb5.h> /* krb5_init_context */
#include <stdlib.h> /* exit */

int main()
{
	krb5_error_code retval;
	krb5_context context;
	
	retval = krb5_init_context(&context);
	if(retval)
	{
		error_message(retval);
		exit(1);
	}
	
	return 0;
}
