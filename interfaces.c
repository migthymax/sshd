#include "interfaces.h"

struct Interface* open_interface(CONST_STRPTR name, int version,void (*error)( CONST_STRPTR)) {
	// ML: TODO: Check that buffers is usfficent, or better calculate needed buffer and allocate it
	char buffer[1024];

	void *args[3]; /* storage for keeping the RawDoFmt arguments */
	args[0] = (void*) name;
	args[1] = (void*) version;

	struct Library *base;
	struct Interface *interface;

	base = IExec->OpenLibrary(name, version);
	if (base != NULL) {
		interface = IExec->GetInterface(base, "main", 1, NULL);
		if (interface != NULL)
			return interface;

		IExec->RawDoFmt("%s has no main interface V1.\n", args, NULL, buffer);
		error(buffer);

		IExec->CloseLibrary(base);
	} else {
		base = IExec->OpenLibrary(name, 0L);
		if (base != NULL) {
			args[2] = (void*) (int) base->lib_Version;
			IExec->RawDoFmt("%s V%lu not found.\nCurrent installed version is V%lu.\n",args, NULL, buffer);
			error(buffer);

			IExec->CloseLibrary(base);
		} else {
			IExec->RawDoFmt("%s V%lu not found. Library is not installed.\n",args, NULL, buffer);
			error(buffer);
		}
	}

	return NULL;
}

void close_interface(struct Interface *interface) {
	if (interface != NULL) {
		struct Library *base = interface->Data.LibBase;

		IExec->DropInterface(interface);
		IExec->CloseLibrary(base);
	}
}
