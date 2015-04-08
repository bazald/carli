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

  flags { "ExtraWarnings", "FloatStrict" } -- Essential to guarantee idential execution of x32 Release to x32 Debug and x64 Debug/Release
  if _ACTION == "vs2013" then
    buildoptions { [[/wd"4005"]], [[/wd"4251"]], [[/wd"4505"]], [[/wd"4996"]] }
    configuration "x32"
      flags { "EnableSSE2" } -- Essential to guarantee idential execution of x32 Release to x32 Debug and x64 Debug/Release
  else
    configuration "Profiling or Release"
      flags { "EnableSSE" }
      buildoptions { "-mfpmath=sse -mmmx -ffp-contract=off" } -- Essential to guarantee idential execution of x32 Release to x32 Debug and x64 Debug/Release
    configuration "*"
      buildoptions { "-Wextra", "-Wnon-virtual-dtor", "-std=c++11", "-pedantic" }
      linkoptions { "-Wl,-rpath,'$$ORIGIN'" }
  end
  if _OPTIONS["clang"] == "true" then
    buildoptions { "-Wno-undefined-bool-conversion" }
  end

  configuration "Debug"
    defines { "_DEBUG", "DEBUG", "debuggable_cast=dynamic_cast", "debuggable_pointer_cast=std::dynamic_pointer_cast" }
    defines { "DEBUG_OUTPUT" }
--     defines { "DISABLE_POOL_ALLOCATOR" }
    flags { "Symbols" }
    TARGETSUFFIX = "_d"
    targetsuffix(TARGETSUFFIX)
  configuration "Profiling"
    defines { "NDEBUG", "debuggable_cast=static_cast", "debuggable_pointer_cast=std::static_pointer_cast" }
    flags { "Symbols", "Optimize" }
    TARGETSUFFIX = "_p"
    targetsuffix(TARGETSUFFIX)
  configuration "Release"
    defines { "NDEBUG", "debuggable_cast=static_cast", "debuggable_pointer_cast=std::static_pointer_cast" }
    flags { "Optimize" }
    targetsuffix "_r"
--    buildoptions { "-flto" }
--    linkoptions { "-flto" }
    TARGETSUFFIX = ""
    targetsuffix(TARGETSUFFIX)

  configuration "windows"
    if _ACTION ~= "vs2013" then
      flags { "StaticRuntime" }
      linkoptions { "-static-libgcc ", "-static-libstdc++" }
    end
  configuration "macosx"
    buildoptions { "-Qunused-arguments" }
    buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
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
        buildoptions { "-Qunused-arguments" }
        buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
        links { "c++", "c++abi", "m", "c", "gcc_s", "gcc" }
    end
  end

  configuration "linux"
    includedirs { "/usr/lib/jvm/java-7-openjdk-amd64/include" }
  configuration "windows"
    includedirs { "C:\\Program Files\\Java\\jdk1.8.0_25\\include",
                  "C:\\Program Files\\Java\\jdk1.8.0_25\\include\\win32" }

  include "src/blocks_world"
  include "src/blocks_world_env"
  include "src/blocks_world_2"
  include "src/blocks_world_2_env"
  include "src/cart_pole"
  include "src/cart_pole_env"
  include "src/carli"
  include "src/console"
  include "src/infinite_mario"
  include "src/mountain_car"
  include "src/mountain_car_env"
  include "src/puddle_world"
  include "src/puddle_world_env"
  include "src/tetris"
  include "src/tetris_env"
