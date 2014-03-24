#ifndef UTILITY_LINKAGE_H
#define UTILITY_LINKAGE_H

#if !defined(_WINDOWS)
#define UTILITY_LINKAGE
#elif !defined(UTILITY_INTERNAL)
#define UTILITY_LINKAGE __declspec(dllimport)
#else
#define UTILITY_LINKAGE __declspec(dllexport)
#endif

#endif
