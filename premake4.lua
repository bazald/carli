solution "carli"
  if _ACTION == "gmake" then
    configurations { "Debug", "Profiling", "Release", "Debug_Clang", "Profiling_Clang", "Release_Clang" }
  else
    configurations { "Debug", "Profiling", "Release" }
  end

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
  buildoptions { "-Wextra", "-std=c++11", "-pedantic" }
  include "src/carli"

  configuration "Debug*"
    defines { "_DEBUG", "DEBUG" }
    flags { "Symbols" }
    targetsuffix "_d"
  configuration "Profiling*"
    defines { "NDEBUG" }
    flags { "Symbols" }
    targetsuffix "_p"
  configuration "Release*"
    defines { "NDEBUG" }
    flags { "Optimize" }
--    buildoptions { "-flto" }
--    linkoptions { "-flto" }

  configuration "windows"
    flags { "StaticRuntime" }
    linkoptions { "-static-libgcc ", "-static-libstdc++" }
  configuration "macosx"
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    linkoptions { "-stdlib=libc++" }

  if _ACTION == "gmake" then
    configuration { "linux" }
      linkoptions { "-Wl,-rpath,/home/bazald/Software/gperftools/lib", "-Wl,-rpath-link,/home/bazald/Software/gperftools/lib" }
    configuration { "linux", "Debug*" }
      links { "tcmalloc" }
    configuration { "linux", "Profiling*" }
      links { "tcmalloc_and_profiler" }
    configuration { "linux", "*clang" }
      buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
      linkoptions { "-stdlib=libc++", "-nodefaultlibs" }
      links { "c++", "c++abi", "m", "c", "gcc_s", "gcc" }
  end
