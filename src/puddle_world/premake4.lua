project "puddle_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../puddle_world_env" }
  links { "env_puddle_world", "carli" }
