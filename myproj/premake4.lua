solution "IT-5102E-16"
    configurations ""
    project "IT-5102E-16"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/linux"

    files({ "*.h", "*.cpp" })
    --includedirs({"/", "includes"})

    flags({"Symbols", "ExtraWarnings"})
    links({})
    --libdirs({"Driver/"})

    buildoptions({"-std=c++11","-Wextra", "-Os", "-s", "-DFREEGLUT_STATIC", "-DGLEW_STATIC"})
    linkoptions({"-std=c++11", "-lpthread", "-lm", "-lGL", "-L/usr/X11R6/lib", "-lGLU", "-lglut", "-lGLEW", "`sdl2-config --cflags --libs`"})
