project "blocks_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../blocks_world_env" }
  links { "env_blocks_world", "carli" }
