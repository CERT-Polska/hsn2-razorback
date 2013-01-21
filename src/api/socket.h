/** @file socket.h
 * Socket API.
 */

#ifndef	RAZORBACK_SOCKET_H
#define RAZORBACK_SOCKET_H

#include "visibility.h"
#include "types.h"
#include <sys/types.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#else //_MSC_VER
#include <sys/socket.h>
#endif //_MSC_VER
#include <openssl/ssl.h>


#ifdef __cplusplus
extern "C" {
#endif

/** Socket Structure
 */
struct Socket
{
#ifdef _MSC_VER
	SOCKET iSocket;
#else
    int iSocket;           ///< The Socket FD
#endif
    struct addrinfo *pAddressInfo;
    bool ssl;
    SSL *sslHandle;
    SSL_CTX *sslContext;
};

/** Starts a socket listening
 * @param *p_sSourceAddress The address
 * @param p_iPort The port
 * @return A new socket or NULL on error.
 */
SO_PUBLIC extern struct Socket * Socket_Listen (const uint8_t * p_sSourceAddress,
                           uint16_t p_iPort);

/** Start a UNIX socket listening 
 * @param p_path Path to the socket file
 * @return A new socket or NULL on error.
 */
SO_PUBLIC extern struct Socket * Socket_Listen_Unix (const char * path);

/** Starts a socket listening
 * @param *p_sSourceAddress The address
 * @param p_iPort The port
 * @return A new socket or NULL on error.
 */
SO_PUBLIC extern struct Socket * SSL_Socket_Connect ( const uint8_t * p_sDestinationAddress,
                            uint16_t p_iPort);

/** Starts a socket from a listening socket
 * @param *p_pSocket the socket
 * @param *p_sListeningSocket the listening socket
 * @return true on success false on error.
 */
SO_PUBLIC extern int Socket_Accept (struct Socket **p_pSocket,
                          const struct Socket *p_sListeningSocket);

/** Starts a connecting socket
 * @param *p_sDestinationAddress the address
 * @param p_iPort the port
 * @return a new socket on success null on failure.
 */
SO_PUBLIC extern struct Socket * Socket_Connect ( const uint8_t * p_sDestinationAddress,
                            uint16_t p_iPort);

/** Close a socket
 * @param *p_pSocket the socket
 */
SO_PUBLIC extern void Socket_Close (struct Socket *p_pSocket);

/** transmits on a socket
 * @param *p_pSocket the socket
 * @param p_iSize the size of data
 * @param *p_sData the data
 * @return true on success false on error
 */
SO_PUBLIC extern bool Socket_Tx (const struct Socket *p_pSocket, uint32_t p_iSize,
                       const uint8_t * p_sData);

/** Formats a string and transmits it on a socket
 * @param p_pSocket the socket to use.
 * @param fmt The printf format string
 * @param ... the format arguments.
 * @return true on success false on error.
 */
SO_PUBLIC extern bool
Socket_Printf (const struct Socket *p_pSocket, const char *fmt, ...);

/** receives on a socket
 * @param *p_pSocket the socket
 * @param p_iSize the size to read
 * @param *p_sData the data
 * @return true on success false on error.
 */
SO_PUBLIC extern bool Socket_Rx (const struct Socket *p_pSocket, uint32_t p_iSize,
                       uint8_t * p_sData);

/** receives on a socket until the terminator is reached
 * @param p_pS the socket
 * @param p_sData the data to read
 * @param p_cTerminator the terminating character
 * @return true on success, false on error
 */
SO_PUBLIC extern size_t Socket_Rx_Until (const struct Socket *p_pS, uint8_t * p_sData,
                             size_t iSize, uint8_t p_cTerminator);

/** determines whether there is any data available
 * @param p_pS the socket
 * @return true if data available, false otherwise
 */
SO_PUBLIC extern bool Socket_ReadyForRead (const struct Socket *p_pS);

#ifdef __cplusplus
}
#endif
#endif // RAZORBACK_SOCKET_H
