project "env_mountain_car"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "MOUNTAIN_CAR_INTERNAL" }

  files { "*.h", "mountain_car_env.cpp" }

  links { "carli" }

project "mountain_car"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "mountain_car.cpp" }

  links { "env_mountain_car", "carli" }
