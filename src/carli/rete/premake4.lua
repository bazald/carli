project "rete"
  kind "SharedLib"
  language "C++"

  targetdir "../../.."

--  if os.get() ~= "windows" then
--    print(os.outputof("make -C grammar"))
--    if _ACTION == "gmake" then
--      prebuildcommands { "+$(MAKE) -C grammar" }
--    else
--      prebuildcommands { "make -C grammar" }
--    end
--  end

  defines { "RETE_INTERNAL" }

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("**.cpp")
    os.mkdir("obj")
    local f = assert(io.open("../obj/scu_rete.cpp", "w"))
    for i, filename in ipairs(matches) do
      f:write("#include \"../"..filename.."\"\n")
    end
    f:close()

    files { "../obj/scu_rete.cpp" }
  else
    files { "**.h", "**.hpp", "**.cpp", "**.lll", "**.yyy" }
  end

  links { "utility" }

  if os.get() == "windows" then
    configuration "Debug"
      postbuildcommands { [[cp rete_d.dll marioai\classes\]] }
    configuration "Profiling"
      postbuildcommands { [[cp rete_p.dll marioai\classes\]] }
    configuration "Release"
      postbuildcommands { [[cp rete.dll marioai\classes\]] }
  end
