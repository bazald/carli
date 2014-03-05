project "carli"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("*.cpp") + os.matchfiles("environments/*.cpp")
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
    files { "*.h", "*.cpp", "environments/*.h", "environments/*.cpp" }
    excludes { "obj/scu.cpp" }
  end

  if os.get() == "windows" then
    prebuildcommands { [[git.bat]] }
  else
    prebuildcommands { [[./git.sh]] }
  end

  libdirs { "../.." }

  configuration "Debug"
    links { "rete_d", "utility_d" }
  configuration "Profiling"
    links { "rete_p", "utility_p" }
  configuration "Release"
    links { "rete_r", "utility_r" }
