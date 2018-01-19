project "env_taxicab"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "TAXICAB_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_taxicab_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_taxicab.dylib" }
