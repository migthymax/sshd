
#include <stdio.h>

#include "interfaces.h"
#include "error.h"

void shellError(CONST_STRPTR message) {
	printf(message);
}

void workbenchError(CONST_STRPTR message) {
	// We only support PPC this can only be started on OS4 V50 or greater, so all the following must work
	struct IntuitionIFace *IIntuition = (struct IntuitionIFace*) open_interface("intuition.library", 50, shellError);
	if (IIntuition != NULL) {

		// Use the improved but not well documented requester class, allows us to show an image, exists since OS2
		Class               *RequesterClass;
		struct ClassLibrary *RequesterBase = IIntuition->OpenClass( "requester.class",50,&RequesterClass );
		if (RequesterBase != NULL ){
			struct Screen *screen = IIntuition->LockPubScreen(NULL);
			if( screen != NULL ) {
				Object * reqobj = IIntuition->NewObject(RequesterClass, NULL,
					REQ_Type,        REQTYPE_INFO,
					REQ_Image,       REQIMAGE_ERROR,
					REQ_TitleText,   "EditIt: Missing system component",
					REQ_BodyText,    message,
					REQ_GadgetText,  "_Ok",
					TAG_END);
				if( reqobj != NULL ) {
					struct orRequest orRequest = { RM_OPENREQ, NULL, NULL, screen };
					IIntuition->IDoMethodA(reqobj, (Msg)&orRequest);
					IIntuition->DisposeObject(reqobj);
					IIntuition->UnlockPubScreen(NULL, screen);
					IIntuition->CloseClass(RequesterBase);

					return;
				}

				IIntuition->UnlockPubScreen(NULL, screen);
			}

			IIntuition->CloseClass(RequesterBase);
		}

		// Fall back to EasyRequet
		struct EasyStruct easyStruct = {
			sizeof(struct EasyStruct), 0,
			"EditIt: Missing system component",
			message,
			"OK"
		};

		IIntuition->EasyRequestArgs( NULL,&easyStruct,NULL,NULL );

		close_interface((struct Interface*) IIntuition);
	}
}

