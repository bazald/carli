project "carli"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  configuration "*"
    files { "**.h", "**.cpp" }
