solution "carli"
  configurations { "Debug", "Release" }

  flags { "ExtraWarnings" }
  buildoptions { "-std=c++11", "-pedantic" }
  include "src/carli"

  configuration "Debug*"
    defines { "_DEBUG", "DEBUG" }
    flags { "Symbols" }
    targetsuffix "_d"
  configuration "Release*"
    defines { "NDEBUG" }
    flags { "Optimize" }

  configuration "windows"
    defines { "_WINDOWS", "WIN32", "_CRT_SECURE_NO_DEPRECATE" }
    flags { "StaticRuntime" }
    platforms { "x32", "x64" }
    linkoptions { "-static-libgcc ", "-static-libstdc++" }
  configuration "macosx"
    defines { "_MACOSX" }
    platforms { "native", "universal" }
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    linkoptions { "-stdlib=libc++" }
    premake.gcc.cc = "clang"
    premake.gcc.cxx = "clang++"
  configuration "linux"
    defines { "_LINUX" }
    platforms { "native" }
