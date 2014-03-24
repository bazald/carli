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
