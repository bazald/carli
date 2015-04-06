project "env_blocks_world_2"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_2_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
