project "utility"
  kind "SharedLib"
  language "C++"

  targetdir "../../.."

  defines { "UTILITY_INTERNAL" }

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("**.cpp")
    os.mkdir("obj")
    local f = assert(io.open("../obj/scu_utility.cpp", "w"))
    for i, filename in ipairs(matches) do
      f:write("#include \"../"..filename.."\"\n")
    end
    f:close()

    files { "../obj/scu_utility.cpp" }
  else
    files { "**.h", "**.hpp", "**.cpp" }
  end

  if os.get() == "windows" then
    configuration "Debug"
      postbuildcommands { [[cp utility_d.dll marioai\classes\]] }
    configuration "Profiling"
      postbuildcommands { [[cp utility_p.dll marioai\classes\]] }
    configuration "Release"
      postbuildcommands { [[cp utility.dll marioai\classes\]] }
  end
