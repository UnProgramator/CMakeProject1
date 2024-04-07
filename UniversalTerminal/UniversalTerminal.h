#pragma once

#ifdef linux

#include <unistd.h>
#include <sys/wait.h>

#elif defined _WIN32

#include <windows.h>

#endif

/*NDEBUG - defined for most compiler when building the release version (no debug) - after MsVC++
*/
#if (!defined NDEBUG) \
	&& (! defined DEBUG_BUILD) 
#define DEBUG_BUILD true
#endif