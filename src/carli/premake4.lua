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

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("**.cpp")
    os.mkdir("obj")
    local f = assert(io.open("obj/scu.cpp", "w"))
    for i, filename in ipairs(matches) do
      if filename ~= "git.cpp" and filename ~= "obj/scu.cpp" then
        f:write("#include \"../"..filename.."\"\n")
      end
    end
    f:close()

    files { "git.cpp", "obj/scu.cpp" }
  else
    files { "**.h", "**.hpp", "**.cpp", "**.lll", "**.yyy" }
    excludes { "obj/scu.cpp" }
  end

  if os.get() == "windows" then
    prebuildcommands { [[git.bat]] }
  else
    prebuildcommands { [[./git.sh]] }
  end
