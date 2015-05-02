project "env_puddle_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "PUDDLE_WORLD_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_puddle_world_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_puddle_world.dylib" }
