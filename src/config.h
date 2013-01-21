/** @file visibility.h
 * Library symbol visibility macro.
 */
#include <stdbool.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include "api/log.h"
#include <stdio.h>

#ifndef _RAZORBACK_VISIBILTY_H
#define _RAZORBACK_VISIBILTY_H

#if defined _WIN32 || defined __CYGWIN__
#  ifdef BUILDING_SO
#    ifdef __GNUC__
#      define SO_PUBLIC __attribute__((dllexport))
#    else
#      define SO_PUBLIC __declspec(dllexport)   // Note: actually gcc seems to also supports this syntax.
#    endif
#  else
#    ifdef __GNUC__
#      define SO_PUBLIC __attribute__((dllimport))
#    else
#      define SO_PUBLIC __declspec(dllimport)   // Note: actually gcc seems to also supports this syntax.
#    endif
#  endif
#  define DLL_LOCAL
#else
#  if __GNUC__ >= 4
#    define SO_PUBLIC __attribute__ ((visibility("default")))
#    define SO_LOCAL  __attribute__ ((visibility("hidden")))
#  else
#    define SO_PUBLIC
#    define SO_LOCAL
#  endif
#endif

#endif /* _RAZORBACK_VISIBILTY_H */


#ifndef ENABLE_ASSERT
#define ENABLE_ASSERT
#endif
#ifndef ENABLE_UNIMPLEMENTED_ASSERT
#define ENABLE_UNIMPLEMENTED_ASSERT
#endif


#define __func__ __FUNCTION__

// String handling functions
//#ifndef snprintf
#define _snprintf snprintf
//#endif

#define _stricmp(X,Y) strcasecmp(X,Y)
#define _strnicmp(X,Y,Z) strncasecmp(X,Y,Z)

// File handling funcitons
//#define access _access
/*
#define R_OK 04
#define W_OK 02
#define F_OK 00
*/
//#define write _write
//#define open _open
//#define close _close
//#define S_IRUSR _S_IREAD
//#define S_IWUSR _S_IWRITE
//#define S_IRGRP 0
//#define S_IROTH 0
/*
#define be64toh(a) _byteswap_uint64(a)
#define be32toh(a) _byteswap_ulong(a)
#define be16toh(a) _byteswap_ushort(a)

#define htobe64(a) _byteswap_uint64(a)
#define htobe32(a) _byteswap_ulong(a)
#define htobe16(a) _byteswap_ushort(a)
*/
//#define inet_ntop InetNtop
//#define inet_pton InetPton
//#define ctime_r ctime

#define HAVE__SNPRINTF_S
#define HAVE__VSNPRINTF_S
#define HAVE_GETADDRINFO

#define HAVE_OPENSSL_AES_H
#define HAVE_OPENSSL_BLOWFISH_H
#define HAVE_OPENSSL_DES_H
#define HAVE_OPENSSL_ECDH_H
#define LIBSSH_EXPORTS
#ifndef WITH_SERVER
#define WITH_SERVER
#endif
#define RZB_PDF_FOX_NUGGET
#define WITH_SFTP
#define ETC_DIR "/etc/hsn2/razorback"
//#define CNC_DEBUG
//#define STOMP_DEBUG


/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* target is linux */
#define C_LINUX 1

/* Debug Mode */
/* #undef DEBUG */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <curl/curl.h> header file. */
#define HAVE_CURL_CURL_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `inet_ntoa' function. */
#define HAVE_INET_NTOA 1

/* Define to 1 if the system has the type `intmax_t'. */
#define HAVE_INTMAX_T 1

/* Define to 1 if the system has the type `intptr_t'. */
#define HAVE_INTPTR_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `magic' library (-lmagic). */
#define HAVE_LIBMAGIC 1

/* Define to 1 if you have the `pcre' library (-lpcre). */
#define HAVE_LIBPCRE 1

/* Define to 1 if you have the `pthread' library (-lpthread). */
#define HAVE_LIBPTHREAD 1

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* Define to 1 if you have the `ssl' library (-lssl). */
#define HAVE_LIBSSL 1

/* Define to 1 if you have the `uuid' library (-luuid). */
#define HAVE_LIBUUID 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if the system has the type `long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <magic.h> header file. */
#define HAVE_MAGIC_H 1

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have a working `mmap' system call. */
#define HAVE_MMAP 1

/* Define to 1 if you have the `munmap' function. */
#define HAVE_MUNMAP 1

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <openssl/evp.h> header file. */
#define HAVE_OPENSSL_EVP_H 1

/* Define to 1 if you have the <pcre.h> header file. */
#define HAVE_PCRE_H 1

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#define HAVE_REALLOC 1

/* Define to 1 if the system has the type `size_t'. */
#define HAVE_SIZE_T 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if the system has the type `ssize_t'. */
#define HAVE_SSIZE_T 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if the system has the type `uint16_t'. */
#define HAVE_UINT16_T 1

/* Define to 1 if the system has the type `uint32_t'. */
#define HAVE_UINT32_T 1

/* Define to 1 if the system has the type `uint8_t'. */
#define HAVE_UINT8_T 1

/* Define to 1 if the system has the type `uintmax_t'. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type `unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define to 1 if you have the <uuid/uuid.h> header file. */
#define HAVE_UUID_UUID_H 1

/* Define if the compiler supports visibility declarations. */
#define HAVE_VISIBILITY 1

/* Inlining status */
#define INLINE inline

/* Performance Stats */
#define RZB_PERF 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

