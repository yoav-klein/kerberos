
#ifndef __UTILS_H__
#define __UTILS_H__

/* 
all these is used so that the user can pass in a 
string describing some security mechanism, and we'll be able
to translate it to the correct mechanism OID
 */

struct mech_oid
{
    char* name;
    gss_OID oid;
};

#pragma weak spnego_mech_oid
gss_OID_desc spnego_mech_oid = { 6, (void*)"\x2b\x06\x01\x05\x05\x02" };
#pragma weak krbv5_mech_oid
gss_OID_desc krbv5_mech_oid = { 9, (void*)"\x2a\x86\x48\x86\xf7\x12\x1\x2\x2" };
#pragma weak available_mechs
struct mech_oid available_mechs[] = {
    { "kerberos", &krbv5_mech_oid },
    { "spnego", &spnego_mech_oid }
};

void print_oid(gss_OID oid);
void print_mechs();

int test_mech_oid(gss_OID mech_oid);
gss_OID test_mech_str(const char *mech_str);

void display_context_flags(int ret_flags);
void display_status(char *message, OM_uint32 major, OM_uint32 minor);

int send_token(int fd, gss_buffer_desc *token);
int recv_token(int fd, gss_buffer_desc *token);



#endif /* __UTILS_H__ */