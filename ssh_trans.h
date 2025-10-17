#pragma once

#define PVE_SSH_VERSION "SSH-2.0"
#define PVE_SSH_VERSION_MIN_SUPPORTED "SSH-2."


struct ssh_client_session {
	struct MinNode node;	
	int socket;
	char *remoteIdentification;
};


struct ssh_client_session *ssh_alloc_session();
void ssh_free_session( struct ssh_client_session *session );