project "env_mountain_car"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "MOUNTAIN_CAR_INTERNAL" }

  files { "**.h", "**.cpp" }

  links { "carli" }

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libenv_mountain_car_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libenv_mountain_car.dylib" }
