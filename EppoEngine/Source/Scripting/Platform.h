#pragma once

#if defined(EP_PLATFORM_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
	#define LOAD_LIB(p) LoadLibraryW(p)
	#define GET_SYM(h, n) GetProcAddress((HMODULE)h, n)
#else
	#include <dlfcn.h>
	#define LOAD_LIB(p) dlopen(p, RTLD_NOW | RTLD_LOCAL)
	#define GET_SYM(h, n) dlsym(h, n)
#endif