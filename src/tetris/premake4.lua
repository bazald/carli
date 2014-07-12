project "env_tetris"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "TETRIS_INTERNAL" }

  files { "*.h", "tetris_env.cpp" }

  links { "carli" }

project "tetris"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "tetris.cpp" }

  links { "env_tetris", "carli" }
