project "carli"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  files { "**.h", "**.cpp" }

  if os.get() == "windows" then
    prebuildcommands { [[git.bat]] }
  else
    prebuildcommands { [[./git.sh]] }
  end
