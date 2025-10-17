#include <proto/bsdsocket.h>
#include <proto/exec.h>

#include <stdlib.h>

#include "ssh_trans.h"

struct ssh_client_session *ssh_alloc_session() {
	struct ssh_client_session *session = malloc( sizeof( struct ssh_client_session) );

	if( session != NULL ) {		
		session->socket = -1;
		session->remoteIdentification = NULL;
	}

	return session;
}

void ssh_free_session( struct ssh_client_session *session ) {
	if( session != NULL ) {
		IExec->Remove( (struct Node *)session );	
		
		if( session->socket >= 0 )
			ISocket->CloseSocket( session->socket );

		if( session->remoteIdentification != NULL )
			free( session->remoteIdentification );

		free( session );
	}
}