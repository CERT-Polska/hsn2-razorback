/** @file metadata.h
 * Metadata wrapper functions.
 */
#ifndef RAZORBACK_METADATA_H
#define RAZORBACK_METADATA_H

#include "visibility.h"
#include "types.h"
#include "ntlv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define Metadata_Add NTLVList_Add
#define Metadata_Get NTLVList_Get

SO_PUBLIC extern bool Metadata_Add_String (struct List *list, uuid_t name, const char *string);
SO_PUBLIC extern bool Metadata_Get_String (struct List *list, uuid_t name, uint32_t *len, const char **string);

SO_PUBLIC extern bool Metadata_Add_IPv4 (struct List *list, uuid_t name, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv4 (struct List *list, uuid_t name, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_IPv6 (struct List *list, uuid_t name, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv6 (struct List *list, uuid_t name, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_Port (struct List *list, uuid_t name, const uint16_t port);
SO_PUBLIC extern bool Metadata_Get_Port (struct List *list, uuid_t name, uint16_t *port);


SO_PUBLIC extern bool Metadata_Add_Filename (struct List *list, const char *name);
SO_PUBLIC extern bool Metadata_Add_Hostname (struct List *list, const char *name);
SO_PUBLIC extern bool Metadata_Add_URI (struct List *list, const char *name);
SO_PUBLIC extern bool Metadata_Add_Path (struct List *list, const char *name);
SO_PUBLIC extern bool Metadata_Add_MalwareName (struct List *list, const char *vendor, const char *name);
SO_PUBLIC extern bool Metadata_Add_Report (struct List *list, const char *text);
SO_PUBLIC extern bool Metadata_Add_CVE (struct List *list, const char *text);
SO_PUBLIC extern bool Metadata_Add_BID (struct List *list, const char *text);
SO_PUBLIC extern bool Metadata_Add_OSVDB (struct List *list, const char *text);
SO_PUBLIC extern bool Metadata_Add_HttpRequest (struct List *list, const char *name);
SO_PUBLIC extern bool Metadata_Add_HttpResponse (struct List *list, const char *name);

SO_PUBLIC extern bool Metadata_Add_IPv4_Source (struct List *list, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv4_Source (struct List *list, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_IPv4_Destination (struct List *list, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv4_Destination (struct List *list, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_IPv6_Source (struct List *list, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv6_Source (struct List *list, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_IPv6_Destination (struct List *list, const uint8_t *addr);
SO_PUBLIC extern bool Metadata_Get_IPv6_Destination (struct List *list, const uint8_t **addr);

SO_PUBLIC extern bool Metadata_Add_Port_Source (struct List *list, const uint16_t port);
SO_PUBLIC extern bool Metadata_Get_Port_Source (struct List *list, uint16_t *port);

SO_PUBLIC extern bool Metadata_Add_Port_Destination (struct List *list, const uint16_t port);
SO_PUBLIC extern bool Metadata_Get_Port_Destination (struct List *list, uint16_t *port);
#ifdef __cplusplus
}
#endif
#endif
