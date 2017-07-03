project "env_advent"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "ADVENT_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_advent_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_advent.dylib" }
