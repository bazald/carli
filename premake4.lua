solution "carli"
  if _ACTION == "gmake" then
    configurations { "Debug", "Release", "Debug_Clang", "Release_Clang" }
  else
    configurations { "Debug", "Release" }
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
  configuration "Release*"
    defines { "NDEBUG" }
    flags { "Optimize" }

  configuration "windows"
    flags { "StaticRuntime" }
    linkoptions { "-static-libgcc ", "-static-libstdc++" }
  configuration "macosx"
    buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
    linkoptions { "-stdlib=libc++" }

  configuration "*"
    if os.get() == "windows" then
      prebuildcommands { [[cmd /c echo #ifndef GIT_H> git.h]],
                         [[cmd /c echo #define GIT_MODIFIED \>> git.h]],
                         [[cmd /c git status | grep -c modified >> git.h]],
                         [[cmd /c echo #define GIT_REVISION \>> git.h]],
                         [[cmd /c git log -n 1 | head -n 1 | sed "s/commit //" >> git.h]],
                         [[cmd /c echo #define GIT_STR_STR(x) #x>> git.h]],
                         [[cmd /c echo #define GIT_STR(x) GIT_STR_STR(x)>> git.h]],
                         [[cmd /c echo #define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)>> git.h]],
                         [[cmd /c echo #define GIT_REVISION_STR GIT_STR(GIT_REVISION)>> git.h]],
                         [[cmd /c echo #endif >>git.h]] }
    else
      if _ACTION == "gmake" then
        prebuildcommands { [[$(shell echo "#ifndef GIT_H" > git.h)]],
                           [[$(shell echo "#define GIT_MODIFIED \\\\" >>git.h)]],
                           [[$(shell git status | grep -c modified >> git.h)]],
                           [[$(shell echo "#define GIT_REVISION \\\\" >> git.h)]],
                           [[$(shell git log -n 1 | head -n 1 | sed 's/commit //' >> git.h)]],
                           [[$(shell echo "#define GIT_STR_STR(x) #x" >> git.h)]],
                           [[$(shell echo "#define GIT_STR(x) GIT_STR_STR(x)" >> git.h)]],
                           [[$(shell echo "#define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)" >> git.h)]],
                           [[$(shell echo "#define GIT_REVISION_STR GIT_STR(GIT_REVISION)" >> git.h)]],
                           [[$(shell echo "#endif" >> git.h)]] }
      elseif _ACTION == "codeblocks" then
        prebuildcommands { [[echo "#ifndef GIT_H" > git.h]],
                           [[echo "#define GIT_MODIFIED \\\\" >>git.h]],
                           [[git status | grep -c modified >> git.h]],
                           [[echo "#define GIT_REVISION \\\\" >> git.h]],
                           [[git log -n 1 | head -n 1 | sed \'s\/commit \/\/\' >> git.h]],
                           [[echo "#define GIT_STR_STR(x) #x" >> git.h]],
                           [[echo "#define GIT_STR(x) GIT_STR_STR(x)" >> git.h]],
                           [[echo "#define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)" >> git.h]],
                           [[echo "#define GIT_REVISION_STR GIT_STR(GIT_REVISION)" >> git.h]],
                           [[echo "#endif" >> git.h]] }
      end
    end

  if _ACTION == "gmake" then
    configuration { "linux", "Debug*" }
      links { "tcmalloc_and_profiler" }
    configuration { "linux", "*clang" }
      buildoptions { "-stdlib=libc++", "-Qunused-arguments" }
      linkoptions { "-stdlib=libc++" }
  end
