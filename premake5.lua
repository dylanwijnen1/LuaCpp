local outputdir = "%{cfg.buildcfg}_%{cfg.architecture}/%{prj.name}"

workspace "LuaCpp"
    startproject "LuaCppUnitTests"

    configurations
    {
        "Debug",
        "Release",
    }

    platforms
    {
        "x64",
        "x86"
    }

project "lua"
    location "thirdparty/lua"
    kind "StaticLib"
    language "C++"

    targetdir("bin/" .. outputdir)
    objdir("temp/" .. outputdir)

    systemversion "latest"

    files { "thirdparty/lua/*.h", "thirdparty/lua/*.c" }

    -- Copy lua.h luaconf.h lualib.h lauxlib.h to include dir.
    print("Creating 'include/' folder for lua.")
    os.mkdir("thirdparty/lua/include/")
    os.copyfile("thirdparty/lua/lua.h", "thirdparty/lua/include/lua.h")
    os.copyfile("thirdparty/lua/lualib.h", "thirdparty/lua/include/lualib.h")
    os.copyfile("thirdparty/lua/lauxlib.h", "thirdparty/lua/include/lauxlib.h")
    os.copyfile("thirdparty/lua/luaconf.h", "thirdparty/lua/include/luaconf.h")
    os.copyfile("thirdparty/lua.hpp", "thirdparty/lua/include/lua.hpp")

project "LuaCpp"
    kind "StaticLib"
    language "C++"

    targetdir("bin/" .. outputdir)
    objdir("temp/" .. outputdir)

    cppdialect "C++17"
    systemversion "latest"

    location "%{prj.name}"

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    -- This allows for <Dragon/...> includes.
    includedirs "%{prj.name}/src"

    filter "platforms:x64"
        architecture "x64"

    filter "platforms:x86"
        architecture "x86"

    filter "configurations:Debug"
        defines "LUACPP_DEBUG"
        symbols "full"
        runtime "Debug"

    filter "configurations:Release"
        defines "LUACPP_RELEASE"
        optimize "On"
        runtime "Release"

    -- Reset filters
    filter {}

    includedirs
    {
        "thirdparty/lua/include"
    }

    links
    {
        "lua"
    }

project "LuaCppUnitTests"
    kind "ConsoleApp"
    language "C++"

    targetdir("bin/" .. outputdir)
    objdir("temp/" .. outputdir)

    cppdialect "C++17"
    systemversion "latest"

    location "%{prj.name}"

    filter "configurations:Debug"
        symbols "full"
        runtime "Debug"

    filter "configurations:Release"
        optimize "On"
        runtime "Release"

    filter{}

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/thirdparty/catch2/include",
        "LuaCpp/src/",
        "thirdparty/lua/include"
    }

    links
    {
        "lua",
        "LuaCpp"
    }
