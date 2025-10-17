#pragma once

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include <intuition/intuition.h>
#include <classes/requester.h>

// ERROR CODES
#define EXIT_RESULT_OK								 0
#define EXIT_RESULT_ILLEGAL_ARGUMENTS				-5
#define EXIT_RESULT_MISSING_LIBRARY					-6
#define EXIT_RESULT_LIBRARY_INIT_FAILED				-7
#define EXIT_RESULT_OUT_OF_MEMORY					-8
#define EXIT_RESULT_CTRL_C							-9
#define EXIT_RESULT_NETWORK_FAILED					-10
#define EXIT_RESULT_SSH_VERSION_NOT_SUPPORTED		-11
#define EXIT_RESULT_NO_SSH_CLIENT					-12

#define EXIT_RESULT_NETWORK_SOCKET_FAILED			-21
#define EXIT_RESULT_NETWORK_DNS_FAILED				-22
#define EXIT_RESULT_NETWORK_CONNECT_FAILED			-23
#define EXIT_RESULT_NETWORK_CLOSED_BY_PEER			-23

void shellError(CONST_STRPTR message);
void workbenchError(CONST_STRPTR message);