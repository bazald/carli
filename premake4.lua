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
    platforms { "native", "x32", "x64" }
  end

  configuration "Debug*"
    defines { "_DEBUG", "DEBUG", "TEST_NASTY_CONDITIONS" }
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
    buildoptions { "-std=c++0x" }

  include "src/carli"
