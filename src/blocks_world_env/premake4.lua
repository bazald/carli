project "env_blocks_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
