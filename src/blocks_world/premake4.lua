project "env_blocks_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_INTERNAL" }

  files { "*.h", "blocks_world_env.cpp" }

  links { "rete", "utility" }

project "blocks_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "blocks_world.cpp" }

  links { "env_blocks_world", "carli", "rete", "utility" }
