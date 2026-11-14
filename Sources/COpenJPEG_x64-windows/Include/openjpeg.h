#ifndef COPENJPEG_MODULE_WRAPPER
#define COPENJPEG_MODULE_WRAPPER

// The vendored openjp2.lib is a static library; without OPJ_STATIC the
// Windows build of OPJ_API expands to __declspec(dllimport) and the link
// fails on missing __imp_ thunks.
#define OPJ_STATIC
#include "openjpeg-2.5/openjpeg.h"

#endif
