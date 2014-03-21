project "puddle_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  links { "carli", "rete", "utility" }
