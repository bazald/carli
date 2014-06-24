#ifndef CARLI_LINKAGE_H
#define CARLI_LINKAGE_H

#if !defined(_WINDOWS)
#define UTILITY_LINKAGE
#define RETE_LINKAGE
#define CARLI_LINKAGE
#define PARSER_LINKAGE
#elif defined(CARLI_INTERNAL)
#define UTILITY_LINKAGE __declspec(dllexport)
#define RETE_LINKAGE __declspec(dllexport)
#define CARLI_LINKAGE __declspec(dllexport)
#define PARSER_LINKAGE __declspec(dllexport)
#else
#define UTILITY_LINKAGE __declspec(dllimport)
#define RETE_LINKAGE __declspec(dllimport)
#define CARLI_LINKAGE __declspec(dllimport)
#define PARSER_LINKAGE __declspec(dllimport)
#endif

#endif
