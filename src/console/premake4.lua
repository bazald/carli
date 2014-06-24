project "console"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "*.h", "*.cpp" }

  links { "env_blocks_world", "carli", "rete", "utility" }
