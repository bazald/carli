project "env_blocks_world_2"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_2_INTERNAL" }

  files { "*.h", "blocks_world_2_env.cpp" }

  links { "carli" }

project "blocks_world_2"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "blocks_world_2.cpp" }

  links { "env_blocks_world_2", "carli" }
