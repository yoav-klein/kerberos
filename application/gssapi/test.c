#include <stdio.h> /* printf */
#include <gssapi.h> /* gss_context */
#include "utils.h" /* display_status */
#include <string.h> /* strcmp */


void print_mech(gss_OID oid)
{
    unsigned int i = 0;
    char *curr_octet = (char*)oid->elements;

    printf("length: %d\n", oid->length);
    printf("content: ");
    for(i = 0; i < oid->length; ++i)
    {
        printf("0x%x ", (unsigned char)*(curr_octet++));
    }
    printf("\n");
}

void print_mechs()
{
    OM_uint32 maj_stat, min_stat;
    gss_OID_set mech_set;
    unsigned int i = 0;

    maj_stat = gss_indicate_mechs(&min_stat, &mech_set);
    if(maj_stat != GSS_S_COMPLETE)
    {
        display_status("gss_indicate_mechs", maj_stat, min_stat);
        return;
    }
    printf("number of mechs: %lu\n", mech_set->count);
    
    for(i = 0; i < mech_set->count; ++i)
    {
        printf("mech number: %d\n", i);
        print_mech(mech_set->elements + i);
    }


    gss_release_oid_set(&min_stat, &mech_set);
}


gss_OID test_available_mech(const char *requested_mech)
{
    unsigned int i = 0;
    for(i = 0; i < sizeof(available_mechs) / sizeof(struct mech_oid); ++i)
    {
        if(!strcmp(available_mechs[i].name, requested_mech))
        {
            return available_mechs[i].oid;
        }
    }
    printf("NOT FOUND\n");
    return NULL;
}

void test_mech()
{
    OM_uint32 maj_stat, min_stat;
    gss_OID_desc spnego = { 6, (void *) "\x2b\x06\x01\x05\x05\x02" };
    gss_OID_set mech_set;
    int present = 0;
    maj_stat = gss_indicate_mechs(&min_stat, &mech_set);
    if(maj_stat != GSS_S_COMPLETE)
    {
        display_status("gss_indicate_mechs", maj_stat, min_stat);
        return;
    }
    
    maj_stat = gss_test_oid_set_member(&min_stat, &spnego, mech_set, &present);
    if(GSS_S_COMPLETE != maj_stat)
    {
        display_status("gss_test_oid_set_member", maj_stat, min_stat);
    }
    printf("mechanism is %s\n", present ? "supported" : "unsuppoerted");
}

int main()
{
    print_mechs();
    test_mech();
    test_available_mech("spnego");
    return 0;
}