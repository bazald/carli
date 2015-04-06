project "tetris"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../tetris_env" }
  links { "env_tetris", "carli" }
