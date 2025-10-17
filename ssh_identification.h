#pragma once

#include "ssh_trans.h"

int ssh_send_identification( struct ssh_client_session *session );
int ssh_read_identification( struct ssh_client_session *session );