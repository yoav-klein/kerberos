/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "GSSAPI"
 * 	found in "../gssapi.asn1"
 */

#ifndef	_InitialContextToken_H_
#define	_InitialContextToken_H_


#include <asn_application.h>

/* Including external dependencies */
#include "MechType.h"
#include <ANY.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* InitialContextToken */
typedef struct InitialContextToken {
	MechType_t	 thisMech;
	ANY_t	 innerContextToken;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InitialContextToken_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InitialContextToken;

#ifdef __cplusplus
}
#endif

#endif	/* _InitialContextToken_H_ */
#include <asn_internal.h>
