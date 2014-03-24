project "carli"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "CARLI_INTERNAL" }

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("*.cpp")
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
    files { "*.h", "*.cpp" }
    excludes { "obj/scu.cpp" }
  end

  if os.get() == "windows" then
    prebuildcommands { [[src\carli\git.bat]] }
  else
    prebuildcommands { [[src/carli/git.sh]] }
  end

--   linkoptions { "-Wl,-rpath,'$$ORIGIN'" }

  links { "rete", "utility" }
