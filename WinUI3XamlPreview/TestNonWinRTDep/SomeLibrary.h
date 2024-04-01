#pragma once

#ifdef TESTNONWINRTDEP_EXPORTS
#define SOME_LIB_API    __declspec(dllexport)
#else
#define SOME_LIB_API    __declspec(dllimport)
#endif

SOME_LIB_API int __stdcall SomeFunction(int x);
