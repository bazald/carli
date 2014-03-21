project "mountain_car"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  links { "carli", "rete", "utility" }
