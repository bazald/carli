project "blocks_world_2"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../blocks_world_2_env" }
  links { "env_blocks_world_2", "carli" }
