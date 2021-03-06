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

  arch = "not-sparc64"
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

    arch = os.outputof("arch")

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
    if arch ~= "sparc64" then
      configuration "Profiling or Release"
        flags { "EnableSSE" }
        buildoptions { "-mfpmath=sse -mmmx -ffp-contract=off" } -- Essential to guarantee idential execution of x32 Release to x32 Debug and x64 Debug/Release
    end
    configuration "*"
      buildoptions { "-Wextra", "-Wnon-virtual-dtor", "-std=c++17", "-pedantic", "-fno-delete-null-pointer-checks" }
    configuration "macosx"
      linkoptions { "-Wl,-rpath,'@loader_path/'" }
    configuration "linux"
      linkoptions { "-Wl,-rpath,'$$ORIGIN'" }
  end
  if _OPTIONS["clang"] == "true" then
    buildoptions { "-Wno-undefined-bool-conversion" }
  end

  configuration "Debug"
--     defines { "NDEBUG", "debuggable_cast=dynamic_cast", "debuggable_pointer_cast=std::dynamic_pointer_cast" }
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
    if arch == "sparc64" then
      buildoptions { "-Os" }
    else
      flags { "Optimize", "Symbols" } -- Errors compiling without Symbols on Mac OS X
    end
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
    if arch == "sparc64" then
      linkoptions { "-Wl,-rpath,/usr/local/lib64",
                    "-Wl,-rpath-link,/usr/local/lib64" }
    else
      configuration { "linux" }
        linkoptions { "-Wl,-rpath,/home/bazald/Software/gperftools/lib",
                      "-Wl,-rpath-link,/home/bazald/Software/gperftools/lib" }
      configuration { "linux", "Debug" }
        links { "tcmalloc" }
      configuration { "linux", "Profiling" }
        links { "tcmalloc_and_profiler" }
      configuration { "linux", "Release" }
        links { "tcmalloc" }
    end
    if _OPTIONS["clang"] == "true" then
      configuration "linux"
        buildoptions { "-Qunused-arguments" }
        buildoptions { "-Wno-deprecated-register", "-Wno-null-conversion", "-Wno-parentheses-equality", "-Wno-unneeded-internal-declaration" }
--         linkoptions { "-stdlib=libc++" }
        links { "c++", "c++abi", "m", "c", "gcc_s", "gcc" }
    end
  end

  configuration "linux"
    includedirs { "/usr/lib/jvm/java-7-openjdk-amd64/include" }
  configuration "macosx"
    includedirs { "/Library/Java/JavaVirtualMachines/jdk1.8.0_45.jdk/Contents/Home/include",
                  "/Library/Java/JavaVirtualMachines/jdk1.8.0_45.jdk/Contents/Home/include/darwin" }
  configuration "windows"
    includedirs { "C:\\Program Files\\Java\\jdk1.8.0_77\\include",
                  "C:\\Program Files\\Java\\jdk1.8.0_77\\include\\win32" }

  include "src/advent"
  include "src/advent_env"
  include "src/blocks_world"
  include "src/blocks_world_env"
  include "src/blocks_world_2"
  include "src/blocks_world_2_env"
--  include "src/cart_pole"
--  include "src/cart_pole_env"
  include "src/carli"
--  include "src/console"
--  include "src/infinite_mario"
  include "src/mountain_car"
  include "src/mountain_car_env"
  include "src/puddle_world"
  include "src/puddle_world_env"
  include "src/sliding_puzzle"
  include "src/sliding_puzzle_env"
--  include "src/stats"
  include "src/taxicab"
  include "src/taxicab_env"
--  include "src/tetris"
--  include "src/tetris_env"
