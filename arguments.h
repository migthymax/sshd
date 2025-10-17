#pragma once

static const char template[] =
	"ADDRESS/K,"
	"PORT/N/K";

enum {
	ADDRESS,
	PORT,
	NUM_ARGS
};
  
/* The array of LONGs where ReadArgs() will store the data from the
command line arguments. C guarantees that all the array entries will be
set to zero.
*/
extern LONG args[NUM_ARGS];