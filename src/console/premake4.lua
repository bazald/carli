project "console"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  links { "env_blocks_world", "env_mountain_car", "env_puddle_world", "env_tetris", "carli" }
