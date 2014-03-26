solution "carli"
  location "."
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
  if _ACTION == "vs2013" then
    buildoptions { "/arch:SSE2 /fp:precise" }
  else
    buildoptions { "-mfpmath=sse -mmmx -msse -msse2 -ffloat-store -ffp-contract=off" } -- Essential to guarantee idential execution of x32 Release to x32 Debug and x64 Debug/Release
    buildoptions { "-Wextra", "-Wnon-virtual-dtor", "-std=c++11", "-pedantic" }
    linkoptions { "-Wl,-rpath,'$$ORIGIN'" }
  end

  configuration "Debug"
    defines { "_DEBUG", "DEBUG", "debuggable_cast=dynamic_cast", "debuggable_pointer_cast=std::dynamic_pointer_cast" }
    defines { "DEBUG_OUTPUT" }
    flags { "Symbols" }
    targetsuffix "_d"
  configuration "Profiling"
    defines { "NDEBUG", "debuggable_cast=static_cast", "debuggable_pointer_cast=std::static_pointer_cast" }
    flags { "Symbols", "Optimize" }
    targetsuffix "_p"
  configuration "Release"
    defines { "NDEBUG", "debuggable_cast=static_cast", "debuggable_pointer_cast=std::static_pointer_cast" }
    flags { "Optimize" }
    targetsuffix "_r"
--    buildoptions { "-flto" }
--    linkoptions { "-flto" }

  configuration "windows"
    flags { "StaticRuntime" }
    if _ACTION ~= "vs2013" then
      linkoptions { "-static-libgcc ", "-static-libstdc++" }
    end
  configuration "macosx"
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
    linkoptions { "-stdlib=libc++" }
  configuration "linux"
    linkoptions { "-Wl,--hash-style=both" }
  configuration "*"
    if _ACTION ~= "vs2013" then
      buildoptions { "-Wno-unused-function" }
    end
    includedirs { "src" }

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

  include "src/blocks_world"
  include "src/infinite_mario"
  include "src/mountain_car"
  include "src/puddle_world"
  include "src/tetris"

  include "src/carli"
  include "src/carli/rete"
  include "src/carli/utility"
