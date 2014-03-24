#ifndef CARLI_LINKAGE_H
#define CARLI_LINKAGE_H

#if !defined(_WINDOWS)
#define CARLI_LINKAGE
#elif !defined(CARLI_INTERNAL)
#define CARLI_LINKAGE __declspec(dllimport)
#else
#define CARLI_LINKAGE __declspec(dllexport)
#endif

#endif
