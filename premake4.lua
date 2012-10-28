solution "carli"
  configurations { "Debug", "Release" }

  if os.get() == "windows" then
    defines { "_WINDOWS", "WIN32", "_CRT_SECURE_NO_DEPRECATE" }
    platforms { "x32", "x64" }
  elseif os.get() == "macosx" then
    premake.gcc.cc = "clang"
    premake.gcc.cxx = "clang++"
    defines { "_MACOSX" }
    platforms { "native", "universal" }
  else
    defines { "_LINUX" }
    platforms { "native" }
  end

  flags { "ExtraWarnings" }

  configuration "Debug*"
    defines { "_DEBUG", "DEBUG" }
    flags { "Symbols" }
    targetsuffix "_d"

  configuration "Release*"
    defines { "NDEBUG" }
    flags { "Optimize" }

  configuration "macosx"
    buildoptions { "-Qunused-arguments" }
    buildoptions { "-stdlib=libc++" }
    linkoptions { "-stdlib=libc++" }

  configuration "linux or macosx"
    buildoptions { "-std=c++0x", "-pedantic" }

  include "src/carli"
