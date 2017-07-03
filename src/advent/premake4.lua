project "advent"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../advent_env" }
  links { "env_advent", "carli" }
