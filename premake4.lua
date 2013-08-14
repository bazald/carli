solution "carli"
  configurations { "Debug", "Profiling", "Release"}

  newoption {
    trigger     = "scu",
    value       = "false",
    description = "Build using the single compilation unit / unity build pattern",
    allowed = {
      { "false", "Normal build" },
      { "true",  "SCU/Unity build" },
    }
  }

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

    newoption {
      trigger     = "clang",
      value       = "false",
      description = "Build using clang++ instead of g++",
      allowed = {
        { "false", "g++" },
        { "true",  "clang++" },
      }
    }
    if _OPTIONS["clang"] == "true" then
      premake.gcc.cc = "clang"
      premake.gcc.cxx = "clang++"
    end
  end

  flags { "ExtraWarnings" }
  buildoptions { "-Wextra", "-Wnon-virtual-dtor", "-std=c++11", "-pedantic" }
  include "src/carli"

  configuration "Debug"
    defines { "_DEBUG", "DEBUG" }
    flags { "Symbols" }
    targetsuffix "_d"
  configuration "Profiling"
    defines { "NDEBUG" }
    flags { "Symbols", "Optimize" }
    targetsuffix "_p"
  configuration "Release"
    defines { "NDEBUG" }
    flags { "Optimize" }
--    buildoptions { "-flto" }
--    linkoptions { "-flto" }

  configuration "windows"
    flags { "StaticRuntime" }
    linkoptions { "-static-libgcc ", "-static-libstdc++" }
  configuration "macosx"
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
    linkoptions { "-stdlib=libc++" }
  configuration "*"
    buildoptions { "-Wno-unused-function" }

  if _ACTION == "gmake" then
--     configuration { "linux" }
--       linkoptions { "-Wl,-rpath,/home/bazald/Software/gperftools/lib",
--                     "-Wl,-rpath-link,/home/bazald/Software/gperftools/lib" }
--     configuration { "linux", "Debug" }
--      links { "tcmalloc" }
--     configuration { "linux", "Profiling" }
--       links { "tcmalloc_and_profiler" }
    if _OPTIONS["clang"] == "true" then
      configuration "linux"
        buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
        buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
        linkoptions { "-stdlib=libc++", "-nodefaultlibs" }
        links { "c++", "c++abi", "m", "c", "gcc_s", "gcc" }
    end
  end
