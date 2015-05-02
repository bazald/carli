project "carli"
  kind "SharedLib"
  language "C++"

  targetdir "../.."

  defines { "CARLI_INTERNAL" }

  if _OPTIONS["scu"] == "true" then
    matches = os.matchfiles("**.cpp")
    os.mkdir("../../obj")
    local f = assert(io.open("../../obj/scu.cpp", "w"))
    for i, filename in ipairs(matches) do
      if filename ~= "git.cpp" and filename ~= "parser/lex.rete.cpp" and filename ~= "parser/rules.tab.cpp" and filename ~= "obj/scu.cpp" then
        f:write("#include \"carli/"..filename.."\"\n")
      end
    end
    f:close()

    files { "git.cpp", "parser/lex.rete.cpp", "parser/rules.tab.cpp", "../../obj/scu.cpp" }
  else
    files { "**.h", "**.hh", "**.cpp", "**.lll", "**.yyy" }
    excludes { "obj/scu.cpp" }
  end

  if os.get() == "windows" then
    prebuildcommands { [[src\carli\git.bat]] }
  else
    prebuildcommands { [[src/carli/git.sh]] }
  end

--   linkoptions { "-Wl,-rpath,'$$ORIGIN'" }

  if os.get() == "windows" then
    configuration "Debug"
      postbuildcommands { [[cp carli_d.dll marioai\classes\]] }
    configuration "Profiling"
      postbuildcommands { [[cp carli_p.dll marioai\classes\]] }
    configuration "Release"
      postbuildcommands { [[cp carli.dll marioai\classes\]] }
  end

  configuration { "macosx", "Debug*" }
    linkoptions { "-install_name @rpath/libcarli_d.dylib" }
  configuration { "macosx", "Release*" }
    linkoptions { "-install_name @rpath/libcarli.dylib" }
