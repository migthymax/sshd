#include "network.h"

#include <stdio.h>
#include <string.h>

// Send full buffer over TCP 
ssize_t send_all( int socket,void *buf,size_t len ) {
    size_t sent = 0;

	while( sent < len ) {
        ssize_t n = ISocket->send( socket,(char*)buf + sent,len - sent,0 );
		
		if( n <= 0 ) {
			return get_socket_errno();
		}

        sent += n;
    }

    return sent;
}

// Receive exactly len bytes */
ssize_t recv_all( int socket,void *buf,size_t len ) {
    size_t recvd = 0;

	while( recvd < len ) {
        ssize_t n = ISocket->recv( socket,(char*)buf + recvd,len - recvd,0 );
        
		if( n <= 0 ) 
			return -1;

        recvd += n;
    }

    return recvd;
}

char *strnstr(const char *haystack, const char *needle, size_t len) {
	int i;
	size_t needle_len;

	if( 0 == (needle_len = strnlen(needle, len) ) ) {
		return (char *)haystack;
	}

	for ( i=0;i <= (int)(len-needle_len);i++ ) {
		if( (haystack[0] == needle[0] ) && ( 0 == strncmp(haystack, needle, needle_len) ) ) {
			return (char *)haystack;
		}

		haystack++;
	}

	return NULL;
}

// Receive up to `len` bytes or up to and including `delimiter`, whichever comes first.
ssize_t recv_delimiter( int socket,void *buf,size_t len,const char *delimiter ) {
	if (delimiter == NULL || delimiter[0] == '\0') {
		return recv_all( socket,buf,len );
	}

	for( size_t recvd = 0;recvd < len; ) {
		ssize_t n = ISocket->recv( socket,(char*)buf + recvd,len - recvd,MSG_PEEK );

		if( n <= 0 ) {
			return -1;
		}

		recvd += (size_t)n;

		void *position = strnstr( buf,delimiter,recvd );

		if( position != NULL ) {
			return ISocket->recv( socket,(char*)buf,(position - buf) + strlen( delimiter ),MSG_WAITALL );
		}
	}

	return ISocket->recv( socket,(char*)buf,len,MSG_WAITALL );
}

int get_socket_errno() {
	int socketErrorNomber = -1;
	if( ISocket->SocketBaseTags( 
		SBTM_SETVAL(SBTC_BREAKMASK),0, /* Disable CTRL-C checking in WaitSelect() */
		SBTM_GETREF(SBTC_ERRNO),&socketErrorNomber,
		TAG_END ) == 0 ) 
	{
		struct TagItem tags[2];
		/* This query will modify the TagItem list. */
		tags[0].ti_Tag	= SBTM_GETVAL( SBTC_ERRNOSTRPTR );
		tags[0].ti_Data	= socketErrorNomber;
		tags[1].ti_Tag	= TAG_END;

		if( ISocket->SocketBaseTagList( tags ) == 0 )
			printf( "ERROR | BSD socket library error number: %lu\nERROR | BSD socket library error: %s\n",socketErrorNomber,(STRPTR)tags[0].ti_Data );
		else
			printf( "ERROR | BSD socket library error number: %lu\n",socketErrorNomber);
	}
	else
		printf( "ERROR | Unable to read BSD socket error number\n" );	

	return socketErrorNomber;
}