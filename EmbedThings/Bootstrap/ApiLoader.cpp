// define a ProcPtr type for each API
#define DO_API(r,n,p)	typedef r (*fp##n##Type) p;
#include "MonoApi.h"

// declare storage for each API's function pointers
#define DO_API(r,n,p)	EXPORT_COREMODULE fp##n##Type n = NULL;
#include "MonoApi.h"
