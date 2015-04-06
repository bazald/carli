project "env_mountain_car"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "MOUNTAIN_CAR_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }
