#ifndef _MERGETIFF_LIBRARY_SETTINGS
#define _MERGETIFF_LIBRARY_SETTINGS

#include <cstdio>
#include <memory>

//Allow users to override the smart pointer type used by the library
#ifndef MERGETIFF_SMART_POINTER_TYPE
#define MERGETIFF_SMART_POINTER_TYPE std::unique_ptr
#endif

//Allow users to specify a logging mechanism for error messages when exception handling is disabled
#ifndef MERGETIFF_ERROR_LOGGER
#define MERGETIFF_ERROR_LOGGER(message) fputs(message, stderr)
#endif

#endif
