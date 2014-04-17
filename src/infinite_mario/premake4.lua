project "infinite_mario"
  kind "SharedLib"
  language "C++"

  targetdir "../.."
  targetsuffix ""

  files { "*.h", "*.cpp" }

  if _ACTION ~= "clean" then
    print(os.outputof("cd ../../marioai && ant"))
    print(os.outputof("cd ../../marioai/classes && javah ch.idsia.ai.agents.ai.JNIAgent"))
  else
    print(os.outputof("cd ../../marioai && ant clean"))
  end

  includedirs { "../../marioai/classes" }

  links { "carli", "rete", "utility" }

  if os.get() == "windows" then
    postbuildcommands { [[cp infinite_mario.dll marioai\classes\]] }
  end
