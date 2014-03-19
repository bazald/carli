project "puddle_world"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  linkoptions { "-Wl,-rpath,'$$ORIGIN/lib'" }

  links { "carli", "rete", "utility" }
