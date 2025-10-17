#include "amissl_name.h"
#include "error.h"
#include "network.h"
#include "ssh_identification.h"

#include <stdio.h>

#define PVE_BUFFER_MAX 255


int ssh_send_identification( struct ssh_client_session *session ) {
	if( session == NULL ) {
		return EXIT_RESULT_ILLEGAL_ARGUMENTS;
	}

	char buffer[PVE_BUFFER_MAX + 1] = {0};
    int strlen = snprintf( buffer,PVE_BUFFER_MAX,"%s-%s\r\n",PVE_SSH_VERSION,getAmiSSLVersionName(AMISSL_CURRENT_VERSION) );

	if( strlen <= PVE_BUFFER_MAX ) {
		return send_all( session->socket,buffer,strlen );
	}

	printf( "Server cannnot send identification '%s' exceeds limit 255 characters!\n",buffer );

	return -1;
}

int ssh_read_identification( struct ssh_client_session *session ) {
	if( session == NULL ) {
		return EXIT_RESULT_ILLEGAL_ARGUMENTS;
	}

	char buffer[PVE_BUFFER_MAX + 1] = {0};

	int received = recv_delimiter( session->socket,buffer,PVE_BUFFER_MAX,"\r\n" );
	if( received >= 4 ) {
		if( strncmp( buffer,"SSH-2.",4 ) == 0 ) {
			printf("Client version: %s ", buffer);

			if( strncmp( buffer,PVE_SSH_VERSION_MIN_SUPPORTED,strlen( PVE_SSH_VERSION_MIN_SUPPORTED) ) == 0 ) {
				printf(" accepted\n" );

				session->remoteIdentification = malloc( received - 2 );
				if( session->remoteIdentification != NULL ) {
					memcpy( session->remoteIdentification,buffer,received - 2 );

					return EXIT_RESULT_OK;
				}
				else {
					printf(" out of memory\n" );

					return EXIT_RESULT_OUT_OF_MEMORY;
				}
			}
			else {
				printf(" not supported\n" );

				return EXIT_RESULT_SSH_VERSION_NOT_SUPPORTED;
			}
		}
		// The else is actually only supported in client mode, to display messages from server
		else {
			printf( "Received message: '%.*s'\n",received,buffer );
		}			
	}
	else if( received < 0 ) {
		return EXIT_RESULT_NETWORK_FAILED;
	}
	
	return EXIT_RESULT_NO_SSH_CLIENT;
}

int ssh_version( int socket ) {
	char buffer[PVE_BUFFER_MAX + 1] = {0};
    int strlen = snprintf( buffer,PVE_BUFFER_MAX,"%s-%s\r\n",PVE_SSH_VERSION,getAmiSSLVersionName(AMISSL_CURRENT_VERSION) );
	if( strlen <= PVE_BUFFER_MAX ) {
		printf("Server version: %s\n",buffer );

		send_all( socket,buffer,strlen );

		memset( buffer,0,PVE_BUFFER_MAX );		
		strlen = ISocket->recv( socket,buffer,PVE_BUFFER_MAX,0 );
		printf( "recoived %d bytes\n",strlen);
		if( strlen >= 0 ) {
			printf("Client version: %s\n", buffer);

			return 0;
		}
	}

	return -1;
}