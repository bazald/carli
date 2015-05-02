project "env_cart_pole"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "CART_POLE_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_cart_pole_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_cart_pole.dylib" }
