
#include <libraries/amisslmaster.h>

#include <proto/amisslmaster.h>
#include <proto/amissl.h>
#include <proto/bsdsocket.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/z.h>

#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <signal.h>
#include <locale.h>
#include <stdio.h>

#include "arguments.h"
#include "error.h"
#include "interfaces.h"
#include "ssh_identification.h"
#include "ssh_algorithms.h"

static const char USED min_stack_cookie[] = "$STACK:102400";

LONG args[NUM_ARGS];

LONG defaultPort	= 22;
char defaultHost[]	= "localhost";
char hostname[MAXHOSTNAMELEN];

struct SocketIFace			*ISocket;
struct AmiSSLMasterIFace	*IAmiSSLMaster;
struct AmiSSLIFace			*IAmiSSL;
struct ZIFace				*IZ;

char kex[128];
char hostkey[128];
char cipher_c2s[128];
char cipher_s2c[128];
char mac_c2s[128];
char mac_s2c[128];
char comp_c2s[32];
char comp_s2c[32];


void setDefaultArguments( LONG args[] ) {
	// PORT/N/K defaults to 22 
	if( ! args[PORT] )
		args[PORT] = defaultPort;

	// TODO: ADDRESS/K 
	/*
	if( args[ADDRESS] ) {
		args[ADDRESS] = (LONG)defaultHost;
		if( ISocket->gethostname( hostname,MAXHOSTNAMELEN ) == 0 ) {
			args[HOST] = (LONG)hostname;

			IExec->DebugPrintF( "[SMB2] INFO | 'start' | Resolved host name '%s'\n",hostname );
		}
		else
			IExec->DebugPrintF( "[SMB2] INFO | 'start' | Failed to resolve host name, using 'localhost'\n" );
	}
	*/
}

int loop( LONG args[],void (*handleError)( CONST_STRPTR) ) {
    int result = EXIT_RESULT_OK;

	setDefaultArguments( args );

	int serverport =  args[ PORT ];

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons( serverport );
	// TODO: Force listening on specifed hsotname
    addr.sin_addr.s_addr = INADDR_ANY;

	int serverSocket = ISocket->socket( AF_INET,SOCK_STREAM,0 );
	if( serverSocket == -1 ) {
		handleError( "Unable to create server socket, Network not available, resp. not setup?\n" );
	
		result = EXIT_RESULT_NETWORK_SOCKET_FAILED;

		goto exit;
	}

	if( ISocket->bind( serverSocket,(struct sockaddr *)&addr,sizeof(addr) ) != 0 ) {
		handleError( "Unable to bind server socket, address already used?\n" );

		goto exit_socket;
	}

	// TODO: allowe more than 1 in backog, format used port
	printf("SSH transport demo listening on port %d...\n",serverport );
    if( ISocket->listen( serverSocket,1 ) != 0 ) {
		handleError( "Unable to listen on server socket\n" );

		goto exit_socket;
	}

	int maxFd = serverSocket;
	struct MinList clientSessions;
	IExec->NewMinList( &(clientSessions) );

	fd_set readFds;

	for( int result = EXIT_RESULT_OK;result == EXIT_RESULT_OK; ) {
		FD_ZERO( &readFds );


		FD_SET( serverSocket,&readFds );
		for( struct ssh_client_session *session = (struct ssh_client_session *)IExec->GetHead(  (struct List *)&(clientSessions) );
			session != NULL;
			session = (struct ssh_client_session *)IExec->GetSucc( (struct Node *)session ) ) {
			FD_SET( session->socket,&readFds );
		}

    	printf( "Listening.....\n" );
		ULONG signals = SIGBREAKF_CTRL_C;
		// Why plus 1, because we want events for all our socket including the last
		int readyDescriptors = ISocket->WaitSelect( maxFd +1,&readFds,NULL,NULL,NULL,&signals );
		if( readyDescriptors == -1 ) {
			printf( "Network Error.\n" );

			result = EXIT_RESULT_NETWORK_FAILED;
		}
		else {
			if( ( signals & SIGBREAKF_CTRL_C ) == SIGBREAKF_CTRL_C ) {
				printf( "Detected CTRL-C.\n" );

				result = EXIT_RESULT_CTRL_C;
			}		
			
			if( FD_ISSET( serverSocket,&readFds ) ) {
				int clientSocket = ISocket->accept( serverSocket,NULL,NULL );
				if( clientSocket != -1 ) {
					printf( "Client connected.\n" );

					// TODO: Check if this is the only client currently in connecting phaae

					struct ssh_client_session *session = ssh_alloc_session();
					if( session != NULL ) {
						session->socket = clientSocket;

						IExec->AddTail( (struct List *)&(clientSessions),(struct Node *)session );

						if( ssh_send_identification( session ) == -1 ) {
							printf( "Failed to send identification.\n" );
	
							result = EXIT_RESULT_NETWORK_FAILED;				
						}
						else {
							maxFd = MAX( maxFd,clientSocket );
						}
					}
					else {
						printf( "Out of memory\n" );

						ISocket->CloseSocket( clientSocket );

						result = EXIT_RESULT_OUT_OF_MEMORY;
					}
				}
				else {
					printf( "Failed to accept client connected.\n" );
				}
			} 

			for( struct ssh_client_session *session = (struct ssh_client_session *)IExec->GetHead(  (struct List *)&(clientSessions) );
				session != NULL;
				session = (struct ssh_client_session *)IExec->GetSucc( (struct Node *)session ) ) {
				if( FD_ISSET( session->socket,&readFds ) ) {
					if( session->remoteIdentification == NULL ) {
						if( ssh_read_identification( session ) < 0 ) {
							printf( "No SSH client\n" );

							// If we free it, it will be removed from the list, so need to handle that
							struct ssh_client_session *prev = (struct ssh_client_session *)IExec->GetPred( (struct Node *)session );
							ssh_free_session( session );
							session = prev;
						}
						else {
							printf( "Client identified as: '%s'\n",session->remoteIdentification );

							// TODO: Send KEXINIT
						}
					}
					else {
						printf( "Read packet!!\n" );

						result = EXIT_RESULT_CTRL_C;
					}
				}
			}
		}
	}

	for( struct ssh_client_session *session = (struct ssh_client_session *)IExec->RemHead(  (struct List *)&(clientSessions) );session != NULL;session = (struct ssh_client_session *)IExec->RemHead(  (struct List *)&(clientSessions) ) ) {		
		ssh_free_session( session );
	}

exit_socket:

	if( ISocket->CloseSocket( serverSocket ) == -1 ) {
		handleError( "Failed to close server socket, don't know how to handle." );
		
		result = EXIT_RESULT_NETWORK_SOCKET_FAILED;

		goto exit;
	}

exit:

	return result;
}

int start( LONG args[],void (*handleError)( CONST_STRPTR) ) {
    int result = EXIT_RESULT_OK;

	IAmiSSLMaster = (struct AmiSSLMasterIFace *)open_interface( "amisslmaster.library",AMISSLMASTER_MIN_VERSION,handleError );
	if( IAmiSSLMaster == NULL ) {
		// TOOD:  inform user which version, where to get 

		result = EXIT_RESULT_MISSING_LIBRARY;

		goto exit;
	}
	
	IAmiSSLMaster->OpenAmiSSLTags( AMISSL_CURRENT_VERSION );

	if( IAmiSSLMaster->OpenAmiSSLTags( AMISSL_CURRENT_VERSION,
		AmiSSL_UsesOpenSSLStructs,		TRUE,
		AmiSSL_InitAmiSSL,				TRUE,
		AmiSSL_GetIAmiSSL,				&IAmiSSL,
		AmiSSL_ErrNoPtr,				__errno(),
		TAG_END ) != 0 ) {
		handleError( "AmiSSL could be initialze.\n" );

		result = EXIT_RESULT_LIBRARY_INIT_FAILED;

		goto exit_amiSSLMaster;
	}

	ISocket = (struct SocketIFace*) open_interface( "bsdsocket.library",4,handleError );
	if( ISocket == NULL ) {
		// TOOD:  inform user which version, where to get 
		result = EXIT_RESULT_MISSING_LIBRARY;

		goto exit_amiSSLMaster;
	}
	
	IZ = (struct ZIFace*) open_interface( "z.library",0,handleError );
	if( IZ == NULL ) {
		// TOOD:  inform user which version, where to get 
		result = EXIT_RESULT_MISSING_LIBRARY;

		goto exit_socket;
	}


	ssh_build_namelist_kex( kex,sizeof( kex ) );
	ssh_build_namelist_hostkey( hostkey,sizeof( hostkey ) );
	ssh_build_namelist_cipher( cipher_c2s,sizeof(cipher_c2s ) );
	ssh_build_namelist_cipher( cipher_s2c,sizeof(cipher_s2c ) );
	ssh_build_namelist_mac( mac_c2s,sizeof(mac_c2s ) );
	ssh_build_namelist_mac( mac_s2c,sizeof(mac_s2c ) );
	ssh_build_namelist_compression( comp_c2s,sizeof( comp_c2s ) );
	ssh_build_namelist_compression( comp_s2c,sizeof( comp_s2c ) );

	// HERE Ready to go
	result = loop( args,handleError );

// UNUSED: exit_z:

	close_interface((struct Interface*) IZ);

exit_socket:		

	close_interface( (struct Interface*)ISocket );

exit_amiSSLMaster:

	close_interface( (struct Interface*)IAmiSSLMaster );

exit:

	return result;
}

int main( int argc, char **argv ) {
    int result = EXIT_RESULT_OK;

	/* Disable c std lib CTRL-C handling. */
	signal( SIGINT,SIG_IGN );

	/* Enable UTF-8 support in newlib. */
	setlocale(LC_CTYPE, "C-UTF-8");

	// Startup errorHandling
	void (*handleError)( CONST_STRPTR) = NULL;
	if( argc == 0 )	{
		handleError = workbenchError;

		// UNUSED: struct WBStartup *workbenchStartup = (struct WBStartup *)argv;

		handleError( "Currently starting from Workbench isn't supported. Start it from the shell." );

		goto exit;
	}
	else {
		handleError = shellError;

		struct RDArgs *rda = IDOS->ReadArgs( template,args,NULL) ;
		if( rda == NULL ) {
			// TOOD: get messane and print vie handleError			
			IDOS->PrintFault( IDOS->IoErr(),NULL );

			result = EXIT_RESULT_ILLEGAL_ARGUMENTS;

			goto exit;
		}

		result = start( args,handleError );

// UNUSED: exit_readArgs:

		IDOS->FreeArgs( rda );
	}

exit:

	IDOS->Printf( "Bye bye, last line of code in main!\n" );
	
	if( result > 0 )
		result = EXIT_RESULT_OK;

	return result;
}