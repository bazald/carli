project "env_blocks_world"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "BLOCKS_WORLD_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_blocks_world_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_blocks_world.dylib" }
