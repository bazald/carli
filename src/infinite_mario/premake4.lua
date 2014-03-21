project "infinite_mario"
  kind "SharedLib"
  language "C++"

  targetdir "../../marioai/classes"
  targetsuffix ""

  files { "*.h", "*.cpp" }

  if _ACTION ~= "clean" then
    print(os.outputof("cd ../../marioai && ant"))
    print(os.outputof("cd ../../marioai/classes && javah ch.idsia.ai.agents.ai.JNIAgent"))
    print(os.outputof("cd ../../marioai/classes && mkdir -p ../../lib && ln -s ../../lib ."))
  else
    print(os.outputof("cd ../../marioai && ant clean"))
  end

  includedirs { "/usr/lib/jvm/java-7-openjdk-amd64/include", "../../marioai/classes" }

  linkoptions { "-Wl,-rpath,../.." }

  links { "carli", "rete", "utility" }
