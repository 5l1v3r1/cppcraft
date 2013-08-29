/*
**
** $Id $
**
** The Lattice (lattice) by remorse@paddylee.com; scalable MMO solutions
** Copyright (C) 2010-2013 Ralph Covelli
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
*/


#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#ifdef __linux__
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <stdlib.h>
	#include <errno.h>
	#include <stdarg.h>
#else
    #include <winsock2.h>
#endif

#include "socket.h"
#include "serversocket.h"

#include "lattice_config.h"
#include "struct.h"
#include "globals.h"
//#include "sched.h"
#include "neighbors.h"
#include "send.h"
#include "macros.h"
//#include "numerics.h"
//#include "users.h"
//#include "server_log.h"

#define CLOSESOCK_BUF 8192

void closesock(server_socket *s) {

    //server_socket *p;

    if (!s) return;

    //if (s->server)
    //   network_server_part(s->server->server_address.x,
    //                       s->server->server_address.y,
    //                       s->server->server_address.z);

/*
    if (!TstFlagClosed(s)) {
        if (sendto_one(s, "CLOSING\n")) return;

        flush_write(s, 1);
    }
*/

//        servercount--;
        del_neighbor(s->coord);

    //sched_delfrom(&sched, s);

    sendq_popall(s);
	
	#ifdef __linux__
		close(s->socket);
	#else
		closesocket(s->socket);
	#endif


    FD_CLR(s->socket, &rready_set);
    FD_CLR(s->socket, &wready_set);
    FD_CLR(s->socket, &rtest_set);
    FD_CLR(s->socket, &wtest_set);

    s->socket = -1;
    s->wlen = 0;
    s->rlen = 0;
    s->sendq_head = NULL;
    s->sendq_tail = NULL;
    s->sendq_length = 0;
    s->sendq_max = 0;
    s->ip.s_addr = 0;
    s->port = 0;
    s->flags = 0;

    s->type = SOCKET_UNKNOWN;

    s->coord.x = 0;
    s->coord.y = 0;
    s->coord.z = 0;

    s->c_port = 0;

    return;

}


void closeallservers(void) {


    int fd;

    for(fd = 0; fd <= maxfd; fd++) {
        if ((fd != input_sock) &&  (FD_ISSET(fd, &rtest_set)) ) {
                closesock(socket_table+fd);
        }
    }


    return;
}


/// cross platform error handling ///

// returns the current error value
socket_error_t sock_getError()
{
	#ifdef __linux__
		return errno;
	#else
		/*
		; // SCOPE //
		{
			int retv = 0, retvlen = sizeof(retv);
			getsockopt(this->socketId, SOL_SOCKET, SO_ERROR, (char*) &retv, &retvlen);
			// avoid circular loops
			if (retv != 0) this->error = retv;
		}
		*/
		return WSAGetLastError();
	#endif
}

// returns TRUE if the provided error can be ignored
int sock_ignoreError(socket_error_t error)
{
	#ifdef __linux__
		return (error == E_WOULDBLOCK || error == EAGAIN || error == EINTR || error == ENOBUFS);
	#else
		return (error == WSAEWOULDBLOCK || error == WSATRY_AGAIN || error == WSAEINTR || error == WSAENOBUFS);
	#endif
}

// returns the current error string, if any
// no side-effects
char* sock_errorString(socket_error_t error)
{
	#ifdef _WIN32
		switch (error)
		{
		case 0:
			return "No error";
		case WSA_INVALID_HANDLE:
			return "Invalid handle";
		case WSA_NOT_ENOUGH_MEMORY:
			return "Not enough memory";
		case WSA_INVALID_PARAMETER:
			return "Invalid parameter";
		case WSA_OPERATION_ABORTED:
			return "Operation aborted";
		case WSA_IO_INCOMPLETE:
			return "IO incomplete (not yet completed)";
		case WSA_IO_PENDING:
			return "IO pending (will complete later)";
		case WSAEINTR:
			return "Interrupted function call";
		case WSAEBADF:
			return "File handle is not valid";
		case WSAEACCES:
			return "Permission denied";
		case WSAEFAULT:
			return "Bad address";
		case WSAEINVAL:
			return "Invalid argument";
		case WSAEMFILE:
			return "Too many open sockets";
		case WSAEWOULDBLOCK:
			return "Operation would block";
		case WSAEINPROGRESS:
			return "Blocking operation already executing";
		case WSAEALREADY:
			return "Operation already in progress";
		case WSAENOTSOCK:
			return "Socket operation on non-socket";
		case WSAEDESTADDRREQ:
			return "Destination address required";
		case WSAEMSGSIZE:
			return "Message size too long";
		case WSAEPROTOTYPE:
			return "Protocol wrong type for socket";
		case WSAENOPROTOOPT:
			return "Unsupported or invalid protocol option";
		case WSAEPROTONOSUPPORT:
			return "Protocol not supported";
		case WSAEOPNOTSUPP:
			return "Operation not supported";
		case WSAEAFNOSUPPORT:
			return "Address family not supported";
		case WSAEADDRINUSE:
			return "Address already in use";
		case WSAEADDRNOTAVAIL:
			return "Address not available";
		case WSAENETDOWN:
			return "Network is down";
		case WSAENETUNREACH:
			return "Network is unreachable";
		case WSAENETRESET:
			return "Network dropped connection on reset";
		case WSAECONNABORTED:
			return "Network caused connection abort";
		case WSAECONNRESET:
			return "Connection reset by peer";
		case WSAENOBUFS:
			return "No buffer space available";
		case WSAEISCONN:
			return "Socket is already connected";
		case WSAENOTCONN:
			return "Socket is not connected";
		case WSAESHUTDOWN:
			return "Socket is shutting down";
		case WSAETOOMANYREFS:
			return "Too many references";
		case WSAETIMEDOUT:
			return "Connection timed out";
		case WSAECONNREFUSED:
			return "Connection refused";
		case WSAELOOP:
			return "Cannot translate name";
		case WSAENAMETOOLONG:
			return "Name too long";
		case WSAEHOSTDOWN:
			return "Host is down";
		case WSAEHOSTUNREACH:
			return "No route to host";
		case WSAENOTEMPTY:
			return "Directory not empty";
		case WSAEPROCLIM:
			return "Too many processes";
		case WSAEUSERS:
			return "User quota exceeded";
		case WSAEDQUOT:
			return "Disk quota exceeded";
		case WSAESTALE:
			return "Stale file handle reference";
		case WSAEREMOTE:
			return "Item is remote";
		case WSASYSNOTREADY:
			return "Network subsystem is unavailable";
		case WSAVERNOTSUPPORTED:
			return "Requested winsock version unsupported";
		case WSANOTINITIALISED:
			return "Winsock not yet initialized";
		case WSAEDISCON:
			return "Graceful shutdown in progress";
		case WSAENOMORE:
			return "No more results";
		case WSAECANCELLED:
			return "Call has been cancelled";
		case WSAEINVALIDPROCTABLE:
			return "Procedure call table is invalid";
		case WSAEINVALIDPROVIDER:
			return "Service provider is invalid";
		case WSAEPROVIDERFAILEDINIT:
			return "Service provider failed to initialize";
		case WSASYSCALLFAILURE:
			return "System call failure";
		case WSASERVICE_NOT_FOUND:
			return "Service not found";
		case WSATYPE_NOT_FOUND:
			return "Class type not found";
		case WSA_E_NO_MORE:
			return "No more results";
		case WSA_E_CANCELLED:
			return "Call was cancelled";
		case WSAEREFUSED:
			return "Database query was refused";
		case WSAHOST_NOT_FOUND:
			return "Host not found";
		case WSATRY_AGAIN:
			return "Nonauthoritative host not found";
		case WSANO_RECOVERY:
			return "This is a nonrecoverable error";
		case WSANO_DATA:
			return "Valid name, no data record of requested type";
		default:
			return "Unknown error";
		}
	#else
		return strerror(error);
	#endif
}