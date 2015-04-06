project "cart_pole"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../cart_pole_env" }
  links { "env_cart_pole", "carli" }
