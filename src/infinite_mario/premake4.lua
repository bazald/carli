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

  includedirs { "C:\\Program Files\\Java\\jdk1.7.0_40\\include", "C:\\Program Files\\Java\\jdk1.7.0_40\\include\\win32",
                "/usr/lib/jvm/java-7-openjdk-amd64/include",
                "../../marioai/classes" }

  links { "carli", "rete", "utility" }

  if os.get() == "windows" then
    postbuildcommands { [[cp infinite_mario.dll marioai\classes\]] }
  end
