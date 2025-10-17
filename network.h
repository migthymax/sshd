#pragma once

#include <proto/bsdsocket.h>

#include <sys/types.h>
#include <stddef.h>

ssize_t send_all( int socket,void *buf,size_t len );
ssize_t recv_all( int socket,void *buf,size_t len );
ssize_t recv_delimiter( int socket,void *buf,size_t len,const char *delimiter );

int get_socket_errno();
