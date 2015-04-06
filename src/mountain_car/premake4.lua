project "mountain_car"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../mountain_car_env" }
  links { "env_mountain_car", "carli" }
