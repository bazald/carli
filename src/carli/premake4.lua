project "carli"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

--  if os.get() ~= "windows" then
--    print(os.outputof("make -C rete/grammar"))
--    if _ACTION == "gmake" then
--      prebuildcommands { "+$(MAKE) -C rete/grammar" }
--    else
--      prebuildcommands { "make -C rete/grammar" }
--    end
--  end

  files { "**.h", "**.hpp", "**.cpp" }

  if os.get() == "windows" then
    prebuildcommands { [[git.bat]] }
  else
    prebuildcommands { [[./git.sh]] }
  end
