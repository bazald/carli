#ifndef RETE_LINKAGE_H
#define RETE_LINKAGE_H

#if !defined(_WINDOWS)
#define RETE_LINKAGE
#elif !defined(RETE_INTERNAL)
#define RETE_LINKAGE __declspec(dllimport)
#else
#define RETE_LINKAGE __declspec(dllexport)
#endif

#endif
