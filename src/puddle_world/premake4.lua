project "env_puddle_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "PUDDLE_WORLD_INTERNAL" }

  files { "*.h", "puddle_world_env.cpp" }

  links { "carli" }

project "puddle_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "puddle_world.cpp" }

  links { "env_puddle_world", "carli" }
