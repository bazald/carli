project "taxicab"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  includedirs { "../taxicab_env" }
  links { "env_taxicab", "carli" }
