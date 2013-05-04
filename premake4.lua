solution "carli"
  configurations { "Debug", "Release" }

  if os.get() == "windows" then
    defines { "_WINDOWS", "WIN32", "_CRT_SECURE_NO_DEPRECATE" }
    platforms { "x32", "x64" }
  elseif os.get() == "macosx" then
    defines { "_MACOSX" }
    platforms { "native", "universal" }
    premake.gcc.cc = "clang"
    premake.gcc.cxx = "clang++"
  else
    defines { "_LINUX" }
    platforms { "native" }
  end

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
    flags { "StaticRuntime" }
    linkoptions { "-static-libgcc ", "-static-libstdc++" }
  configuration "macosx"
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    linkoptions { "-stdlib=libc++" }
