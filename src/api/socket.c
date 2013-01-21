#include "../config.h"
#include "debug.h"
#include "socket.h"
#include "log.h"

#ifdef _MSC_VER
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <io.h>
#pragma comment(lib, "Ws2_32.lib")
#define OPT_CAST const char *
#else
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#define OPT_CAST void *
#endif

#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>




/* controls the amount sent per call to read or write */
#define	MAXRWSIZE	1024
static void
Socket_Destroy (struct Socket *p_pSocket)
{
    if (p_pSocket->pAddressInfo != NULL)
        freeaddrinfo (p_pSocket->pAddressInfo);
    free(p_pSocket);
}

static bool
Socket_CopyAddress (struct Socket *p_pDest, const struct Socket *p_pSource)
{
    ASSERT (p_pDest->pAddressInfo == NULL);
    ASSERT (p_pSource->pAddressInfo != NULL);

    if ((p_pDest->pAddressInfo = (struct addrinfo *)calloc(1,sizeof(struct addrinfo))) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new address info", __func__);
        return false;
    }

    p_pDest->pAddressInfo->ai_flags= p_pSource->pAddressInfo->ai_flags;
    p_pDest->pAddressInfo->ai_family= p_pSource->pAddressInfo->ai_family;
    p_pDest->pAddressInfo->ai_socktype= p_pSource->pAddressInfo->ai_socktype;
    p_pDest->pAddressInfo->ai_protocol= p_pSource->pAddressInfo->ai_protocol;
    p_pDest->pAddressInfo->ai_addrlen= p_pSource->pAddressInfo->ai_addrlen;
    
    p_pDest->pAddressInfo->ai_next= NULL; // No next when we clone it.

    p_pDest->pAddressInfo->ai_canonname = NULL;
    

    if ((p_pDest->pAddressInfo->ai_addr = (struct sockaddr *)
         malloc (p_pSource->pAddressInfo->ai_addrlen)) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new address", __func__);
        return false;
    }

    memcpy (p_pDest->pAddressInfo->ai_addr, p_pSource->pAddressInfo->ai_addr,
            p_pSource->pAddressInfo->ai_addrlen);

    return true;
}

static bool
SocketAddress_Initialize (struct Socket *p_pSocket,
                          const uint8_t * p_sAddress, uint16_t p_iPort)
{
	struct addrinfo l_aiHints;
    uint8_t l_sPortAsString[32];
	int ret;

    ASSERT (p_pSocket->pAddressInfo == NULL);

    if (p_pSocket->pAddressInfo != NULL)
    {
        rzb_log(LOG_ERR, "%s: Double address init", __func__);
        return false;
    }
    
    sprintf ((char *) l_sPortAsString, "%i", p_iPort);
    memset (&l_aiHints, 0, sizeof (struct addrinfo));
    
    l_aiHints.ai_family = AF_UNSPEC;
    l_aiHints.ai_socktype = SOCK_STREAM;
    l_aiHints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	l_aiHints.ai_protocol = IPPROTO_TCP;
        
    if ((ret = getaddrinfo
        ((const char *) p_sAddress, (const char *) l_sPortAsString,
         &l_aiHints, &p_pSocket->pAddressInfo)) != 0)
    {

#ifdef _MSC_VER
		rzb_log(LOG_ERR, "Failed to get address info: %S, %d, %S", p_sAddress, ret, gai_strerror(ret));
#else
        rzb_perror
            ("Failed to get address information in SocketAddress_Initialize: %s");
#endif
        p_pSocket->pAddressInfo = NULL;
        return false;
    }

    return true;
}

SO_PUBLIC struct Socket *
Socket_Listen (const unsigned char *p_sSourceAddress, uint16_t p_iPort)
{
    struct Socket *l_pSocket;
    int l_iOn = 1;

	ASSERT (p_sSourceAddress != NULL);

    if ((l_pSocket = (struct Socket *)calloc (1, sizeof (struct Socket))) == NULL) 
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new socket", __func__);
        return NULL;
    }
   
    if (!SocketAddress_Initialize
        (l_pSocket, p_sSourceAddress, p_iPort))
    {
        rzb_log (LOG_ERR,
                 "%s: failed due to failure of SocketAddress_Initialize", __func__);
        Socket_Destroy(l_pSocket);
        return NULL;
    }

    if ((l_pSocket->iSocket =
         socket (l_pSocket->pAddressInfo->ai_family, l_pSocket->pAddressInfo->ai_socktype,
                 l_pSocket->pAddressInfo->ai_protocol)) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of socket call: %s");
        return NULL;
    }

    if (setsockopt( l_pSocket->iSocket, SOL_SOCKET, SO_REUSEADDR, (OPT_CAST)&l_iOn, sizeof(l_iOn) ) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of setsockopt: %s");
        return NULL;
    }
    if (bind
        (l_pSocket->iSocket,
         l_pSocket->pAddressInfo->ai_addr,
         l_pSocket->pAddressInfo->ai_addrlen) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of bind call: %s");
        return NULL;
    }

    if (listen (l_pSocket->iSocket, SOMAXCONN) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of listen call: %s");
        return NULL;
    }

    // done
    return l_pSocket;
}

SO_PUBLIC struct Socket *
Socket_Listen_Unix (const char *path)
{
#ifdef _MSC_VER
	ASSERT(false);
	return NULL;
#else
    struct Socket *l_pSocket;
    struct sockaddr_un *server;


    if((server = calloc(1, sizeof(struct sockaddr_un))) == NULL)
        return NULL;
    server->sun_family = AF_UNIX;
    strncpy (server->sun_path, path, sizeof (server->sun_path));
    server->sun_path[sizeof (server->sun_path) - 1] = '\0';

    if ((l_pSocket = (struct Socket *)calloc (1, sizeof (struct Socket))) == NULL) 
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new socket", __func__);
        return NULL;
    }
    if ((l_pSocket->pAddressInfo = (struct addrinfo *)calloc(1,sizeof(struct addrinfo))) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new address info", __func__);
        return false;
    }

    l_pSocket->pAddressInfo->ai_family=AF_UNIX;
    l_pSocket->pAddressInfo->ai_next= NULL;
    l_pSocket->pAddressInfo->ai_canonname = NULL;
    l_pSocket->pAddressInfo->ai_addrlen= sizeof(struct sockaddr_un);
    l_pSocket->pAddressInfo->ai_addr = (struct sockaddr *)server;

    if ((l_pSocket->iSocket = socket (AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of socket call: %s");
        return NULL;
    }

    if (bind
        (l_pSocket->iSocket,
         l_pSocket->pAddressInfo->ai_addr,
         l_pSocket->pAddressInfo->ai_addrlen) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen_Unix failed due to failure of bind call: %s");
        return NULL;
    }

    if (listen (l_pSocket->iSocket, SOMAXCONN) == -1)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror ("Socket_Listen failed due to failure of listen call: %s");
        return NULL;
    }

    // done
    return l_pSocket;
#endif
}

/* Returns: 0 = timeout, -1 = error, 1 = ok */
SO_PUBLIC int
Socket_Accept (struct Socket **p_pSocket,
               const struct Socket *p_sListeningSocket)
{
    struct Socket *l_pSocket;
    fd_set l_fdsRead;
    struct timeval l_tTimeout;

    ASSERT (p_pSocket != NULL);
    ASSERT (p_sListeningSocket != NULL);

    if ((l_pSocket = (struct Socket *)calloc(1, sizeof (struct Socket))) == NULL)
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new socket", __func__);
        return -1;
    }

    Socket_CopyAddress (l_pSocket, p_sListeningSocket);

    l_tTimeout.tv_sec = 0;
    l_tTimeout.tv_usec = 10000;
    FD_ZERO (&l_fdsRead);
    FD_SET (p_sListeningSocket->iSocket, &l_fdsRead);
    if (select (p_sListeningSocket->iSocket +1, &l_fdsRead, NULL, NULL, &l_tTimeout) < 0)
    {
        Socket_Destroy (l_pSocket);
        rzb_perror
            ("Socket_Accept failed due to failure of accept call: %s");
        return -1;
    }
    
    {
        // check for error
        if ((l_pSocket->iSocket =
             accept (p_sListeningSocket->iSocket,
                     l_pSocket->pAddressInfo->ai_addr,
                     &l_pSocket->pAddressInfo->ai_addrlen)) == -1)
        {
            Socket_Destroy (l_pSocket);
            rzb_perror
                ("Socket_Accept failed due to failure of accept call: %s");
            return -1;
        }
        *p_pSocket = l_pSocket;
        // ok
        return 1;
    }
    // timeout
    return 0;
}

SO_PUBLIC struct Socket *
Socket_Connect (const unsigned char *p_sDestinationAddress, uint16_t p_iPort)
{
    struct addrinfo *cur = NULL;
    struct Socket *l_pSocket;

#ifdef _MSC_VER
	DWORD nTimeout = 5000;
#endif

	ASSERT (p_sDestinationAddress != NULL);

    if ((l_pSocket = (struct Socket *)calloc (1, sizeof (struct Socket))) == NULL) 
    {
        rzb_log(LOG_ERR, "%s: Failed to allocate new socket", __func__);
        return NULL;
    }
    l_pSocket->ssl =false;

    if (!SocketAddress_Initialize
        (l_pSocket, p_sDestinationAddress, p_iPort))
    {
        rzb_log (LOG_ERR,
                 "%s: failed due to failure of SocketAddress_Initialize", __func__);
        return NULL;
    }
    
    cur = l_pSocket->pAddressInfo;
    while (cur != NULL)
    {
#ifdef _MSC_VER
		if ((l_pSocket->iSocket = socket (cur->ai_family, cur->ai_socktype, cur->ai_protocol)) ==INVALID_SOCKET)
#else
        if ((l_pSocket->iSocket = socket (cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1)
#endif
        {
            rzb_perror
                ("Socket_Connect failed due to failure of socket call: %s");
            cur = cur->ai_next;
            continue;
        }

#ifdef _MSC_VER
		setsockopt(l_pSocket->iSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nTimeout, sizeof(int));
#endif

#ifdef _MSC_VER
		if (connect(l_pSocket->iSocket, cur->ai_addr, cur->ai_addrlen) == SOCKET_ERROR)
#else
        if (connect(l_pSocket->iSocket, cur->ai_addr, cur->ai_addrlen) == -1)
#endif
        {
            rzb_perror
                ("Socket_Connect failed due to failure of connect call: %s");
            cur = cur->ai_next;
#ifdef _MSC_VER
			closesocket(l_pSocket->iSocket);
#else
            close(l_pSocket->iSocket);
#endif
            continue;
        }
        return l_pSocket;
    }

    rzb_log(LOG_ERR,"%s: All possible hosts exhausted", __func__);
    Socket_Close(l_pSocket);
    return NULL;
}

SO_PUBLIC struct Socket * 
SSL_Socket_Connect ( const uint8_t * destination, uint16_t port)
{
    struct Socket *socket;
    if ((socket = Socket_Connect(destination, port)) == NULL)
    {
        return NULL;
    }
    socket->ssl =true;
    if ((socket->sslContext = SSL_CTX_new(SSLv23_client_method())) == NULL)
    {
        Socket_Close(socket);
        return NULL;
    }
    if ((socket->sslHandle = SSL_new (socket->sslContext)) == NULL) 
    {
        Socket_Close(socket);
        return NULL;
    }
    
    if (!SSL_set_fd (socket->sslHandle, socket->iSocket))
    {
        Socket_Close(socket);
        return NULL;
    }

    // Initiate SSL handshake
    if (SSL_connect (socket->sslHandle) != 1)
    {
        Socket_Close(socket);
        return NULL;
    }
    return socket;
    
}
SO_PUBLIC void
Socket_Close (struct Socket *p_pSocket)
{
    ASSERT (p_pSocket != NULL);

#ifdef _MSC_VER
    closesocket(p_pSocket->iSocket);
#else
    close(p_pSocket->iSocket);
#endif
    if (p_pSocket->ssl)
    {
        if (p_pSocket->sslHandle)
        {
            SSL_shutdown (p_pSocket->sslHandle);
            SSL_free (p_pSocket->sslHandle);
        }
        if (p_pSocket->sslContext)
            SSL_CTX_free (p_pSocket->sslContext);
    }
    Socket_Destroy (p_pSocket);

    // done
}

SO_PUBLIC bool
Socket_Tx (const struct Socket *p_pSocket, uint32_t p_iSize,
           const uint8_t * p_sData)
{
    int l_iAmountRemaining;
    int l_iAmountSent;
    int l_iAmountToSendThisTime;
    int l_iAmountSentThisTime;

	ASSERT (p_pSocket != NULL);
    ASSERT (p_sData != NULL);

    l_iAmountRemaining = p_iSize;
    l_iAmountSent = 0;

    while (l_iAmountRemaining > 0)
    {
        l_iAmountToSendThisTime = l_iAmountRemaining;
        if (l_iAmountToSendThisTime > MAXRWSIZE)
        {
            l_iAmountToSendThisTime = MAXRWSIZE;
        }
        if (p_pSocket->ssl)
        {
            l_iAmountSentThisTime =
                SSL_write (p_pSocket->sslHandle, p_sData + l_iAmountSent,
                       l_iAmountToSendThisTime);
        }
        else
        {
            l_iAmountSentThisTime =
                send (p_pSocket->iSocket, (const char *)p_sData + l_iAmountSent,
                       l_iAmountToSendThisTime, 0);
        }
        if (l_iAmountSentThisTime < 1)
        {
            rzb_perror ("Socket_Tx failed due to failure of write call: %s");
            return false;
        };
        l_iAmountSent += l_iAmountSentThisTime;
        l_iAmountRemaining -= l_iAmountSentThisTime;
    }
    // done
    return true;
}

SO_PUBLIC bool
Socket_Printf (const struct Socket *p_pSocket, const char *fmt, ...)
{
    char *buffer = NULL;
    va_list argp;
    va_start(argp, fmt);
    if(vasprintf(&buffer, fmt, argp) == -1)
    {
        va_end(argp);
        return false;
    }
    va_end(argp);
    if (Socket_Tx(p_pSocket, strlen(buffer), (uint8_t *)buffer))
    {
        free(buffer);
        return true;
    }
    else
    {
        free(buffer);
        return false;
    }
}


SO_PUBLIC bool
Socket_Rx (const struct Socket * p_pSocket, uint32_t p_iSize,
           uint8_t * p_sData)
{
    int l_iAmountRemaining;
    int l_iAmountReceived;
    int l_iAmountToReadThisTime;
    int l_iAmountReadThisTime;

	ASSERT (p_pSocket != NULL);
    ASSERT (p_iSize > 0);
    ASSERT (p_sData != NULL);

    l_iAmountRemaining = p_iSize;
    l_iAmountReceived = 0;

    while (l_iAmountRemaining > 0)
    {
        l_iAmountToReadThisTime = l_iAmountRemaining;
        if (l_iAmountToReadThisTime > MAXRWSIZE)
            l_iAmountToReadThisTime = MAXRWSIZE;
        if (p_pSocket->ssl)
        {
            l_iAmountReadThisTime =
                SSL_read (p_pSocket->sslHandle, p_sData + l_iAmountReceived,
                  l_iAmountToReadThisTime);
        }
        else
        {
            l_iAmountReadThisTime =
                recv (p_pSocket->iSocket, (char *)p_sData + l_iAmountReceived,
                      l_iAmountToReadThisTime, 0);
        }

        if (l_iAmountReadThisTime < 1)
        {
#ifdef _MSC_VER
			if (WSAGetLastError() == WSAETIMEDOUT)
				errno = EINTR;
#endif
            if (errno != EINTR && errno != EAGAIN) 
                rzb_perror ("Socket_Rx failed due to failure of read call: %s");
            if (errno == 0)
                errno = EPIPE;

            return false;
        }
        l_iAmountReceived += l_iAmountReadThisTime;
        l_iAmountRemaining -= l_iAmountReadThisTime;
    }

    // done
    return true;
}

SO_PUBLIC size_t
Socket_Rx_Until (const struct Socket * p_pSocket, uint8_t * p_sData,
                 size_t iSize, uint8_t p_cTerminator)
{
    // counter
    uint32_t l_iOffset = 0;
 
	ASSERT (p_pSocket != NULL);
    ASSERT (p_sData != NULL);

	while (l_iOffset < iSize) 
    {
        if (!Socket_Rx (p_pSocket, 1, &p_sData[l_iOffset]))
        {
            if (errno != EINTR && errno != EAGAIN)
                rzb_log (LOG_ERR, "%s: failed due to failure of Socket_Rx", __func__);

            return 0;
        }
        if (p_sData[l_iOffset++] == p_cTerminator)
            return l_iOffset;
    }

    // done
    return 0;
}

SO_PUBLIC bool
Socket_ReadyForRead (const struct Socket * p_pSocket)
{
    // temporary variables
    struct timeval l_tvTimeOut;
    fd_set l_fdsReadEvents;
    fd_set l_fdsWriteEvents;
    fd_set l_fdsExceptEvents;

	ASSERT (p_pSocket != NULL);

    // setup descriptors
    FD_ZERO (&l_fdsReadEvents);
    FD_SET (p_pSocket->iSocket, &l_fdsReadEvents);
    FD_ZERO (&l_fdsWriteEvents);
    FD_ZERO (&l_fdsExceptEvents);

    // specify timeout
    l_tvTimeOut.tv_sec = 0;
    l_tvTimeOut.tv_usec = 1000; // 1 ms time-out

    // return true if it does not timeout
    return (select
            (p_pSocket->iSocket + 1, &l_fdsReadEvents, &l_fdsWriteEvents,
             &l_fdsExceptEvents, &l_tvTimeOut) != 0);
}


