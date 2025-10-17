#pragma once

#include <proto/exec.h>

struct Interface* open_interface(CONST_STRPTR name, int version,void (*error)( CONST_STRPTR));
void close_interface(struct Interface *interface);