project "env_tetris"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "TETRIS_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_tetris_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_tetris.dylib" }
