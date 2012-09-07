project "carli"
  kind "ConsoleApp"
  language "C++"

  targetdir "../.."

  configuration "linux or macosx"
    buildoptions { "-ffast-math", "-fpch-preprocess", "-Wall" }

  configuration "*"
    flags { "ExtraWarnings" }

    files { "**.h", "**.cpp" }
