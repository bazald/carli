project "env_puddle_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "PUDDLE_WORLD_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
