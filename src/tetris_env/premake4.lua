project "env_tetris"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "TETRIS_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
