project "mountain_car"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  linkoptions { "-Wl,-rpath,'$$ORIGIN/lib'" }

  links { "carli", "rete", "utility" }
