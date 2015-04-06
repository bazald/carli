project "env_cart_pole"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "CART_POLE_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
