project "env_blocks_world_2"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_INTERNAL" }

  files { "*.h", "blocks_world_env.cpp" }

  links { "carli" }

project "blocks_world_2"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "blocks_world.cpp" }

  links { "env_blocks_world_2", "carli" }
