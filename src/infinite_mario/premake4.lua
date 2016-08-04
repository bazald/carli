project "infinite_mario"
  kind "SharedLib"
  language "C++"

  targetdir "../.."
  targetsuffix ""

  files { "**.h", "**.cpp" }

  local subp = nil
  local output = nil
  if _ACTION ~= "clean" then
    subp = io.popen("cd ../../marioai && ant")
    output = subp:read('*a')
    subp:close()
    print(output)

    subp = io.popen("cd ../../marioai/classes && javah ch.idsia.ai.agents.ai.JNIAgent")
    output = subp:read('*a')
    subp:close()
    print(output)
  else
    subp = io.popen("cd ../../marioai && ant clean")
    output = subp:read('*a')
    subp:close()
    print(output)
  end

  includedirs { "../../marioai/classes" }

  links { "carli" }

  if os.get() == "windows" then
    postbuildcommands { [[cp infinite_mario.dll marioai\classes\]] }
  end
