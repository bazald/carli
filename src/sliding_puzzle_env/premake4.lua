project "env_sliding_puzzle"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "SLIDING_PUZZLE_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_sliding_puzzle_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_sliding_puzzle_2.dylib" }
