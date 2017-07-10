project "stats"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  links { "env_blocks_world_2", "carli" }
