project "sliding_puzzle"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../sliding_puzzle_env" }
  links { "env_sliding_puzzle", "carli" }
