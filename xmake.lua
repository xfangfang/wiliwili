add_rules("mode.debug", "mode.release")


option("sw")
    set_default(false)
    set_showmenu(true)
option_end()

option("winrt")
    set_default(false)
    set_showmenu(true)
option_end()

option("window")
    set_default("glfw")
    set_showmenu(true)
option_end()

option("driver")
    set_default("opengl")
    set_showmenu(true)
option_end()

if is_plat("windows") then
    add_cxflags("/utf-8")
    set_languages("c++20")
    if is_mode("release") then
        set_optimize("faster")
    end
else
    set_languages("c++17")
end

package("borealis")
    set_sourcedir("../borealis")
    add_configs("window", {description = "use window lib", default = "glfw", type = "string"})
    add_configs("driver", {description = "use driver lib", default = "opengl", type = "string"})
    add_configs("winrt", {description = "use winrt api", default = false, type = "boolean"})
    add_deps(
        "nanovg",
        "yoga",
        "nlohmann_json",
        "fmt",
        "tweeny",
        "stb",
        "tinyxml2"
    )
    add_includedirs("include")
    if is_plat("windows") then
        add_includedirs("include/compat")
    end
    on_load(function (package)
        local window = package:config("window")
        local driver = package:config("driver")
        local winrt = package:config("winrt")
        if window == "glfw" then
            package:add("deps", "xfangfang_glfw")
        elseif window == "sdl" then
            package:add("deps", "sdl2")
        end
        if driver == "opengl" then
            package:add("deps", "glad")
        elseif driver == "d3d11" then
            package:add("syslinks", "d3d11")
        end
        if winrt then
            package:add("syslinks", "windowsapp")
        end
    end)
    on_install(function (package)
        local configs = {}
        local window = package:config("window")
        local driver = package:config("driver")
        configs["window"] = window
        configs["driver"] = driver
        local winrt = package:config("winrt")
        configs["winrt"] = winrt and "y" or "n"
        import("package.tools.xmake").install(package, configs)
        os.cp("library/include/*", package:installdir("include").."/")
        os.rm(package:installdir("include/borealis/extern"))
        os.cp("library/include/borealis/extern/libretro-common", package:installdir("include").."/")
    end)
package_end()

package("mpv")
    if is_plat("windows", "mingw") then
        set_urls("https://github.com/zhongfly/mpv-winbuild/releases/download/2023-04-24/mpv-dev-x86_64-v3-20230424-git-4fd0a39.7z")
        add_versions("20230424", "e29d9749a7a77c78f308b1fb82a322cbaf4a2dc79369228ae4cba2d8c3fd2053")
    end
    add_links("mpv")
    on_install("windows", "mingw", function (package)
        import("detect.sdks.find_vstudio")
        os.cp("include/*", package:installdir("include").."/")
        os.cp("*.a", package:installdir("lib").."/")
        os.cp("*.dll", package:installdir("bin").."/")

        -- 从 dll 里导出函数为 lib 文件，预编译自带 def 文件格式不正确，没法导出 lib
        if os.isfile("mpv.def") then
            local def_context = io.readfile("mpv.def")
            if not def_context:startswith("EXPORTS") then
                io.writefile("mpv.def", format("EXPORTS\n%s", def_context))
            end
        end
        local vs = find_vstudio()["2022"]["vcvarsall"]["x64"]
        local libExec = path.join(
            vs["VSInstallDir"],
            "VC",
            "Tools",
            "MSVC",
            vs["VCToolsVersion"],
            "bin",
            "HostX64",
            "x64",
            "lib.exe"
        )
        os.execv(libExec, {"/name:libmpv-2.dll", "/def:mpv.def", "/out:mpv.lib", "/MACHINE:X64"})
        os.cp("*.lib", package:installdir("lib").."/")
        os.cp("*.exp", package:installdir("lib").."/")
    end)
package_end()

if get_config("winrt") then
    add_requireconfs("**.sdl2", {configs={shared=true,winrt=true}})
    add_requires("borealis", {configs={window="sdl",driver=get_config("driver"),winrt=true}})
    add_requireconfs("**.curl", {configs={winrt=true}})
else
    add_requires("borealis", {configs={window=get_config("window"),driver=get_config("driver")}})
end
add_requires("mpv", {configs={shared=true}})
add_requires("cpr")
add_requires("lunasvg")
add_requires("opencc")
add_requires("pystring")
add_requires("qr-code-generator", {configs={cpp=true}})

target("wiliwili")
    add_includedirs("wiliwili/include", "wiliwili/include/api")
    add_files("wiliwili/source/**.cpp")
    add_defines("BRLS_RESOURCES=\"./resources/\"")
    if get_config("window") == 'sdl' then
        add_defines("__SDL2__=1")
    else
        add_defines("__GLFW__=1")
    end
    if get_config("winrt") then
        add_defines("__WINRT__=1")
    end
    if get_config("sw") then
        add_defines("MPV_SW_RENDER=1")
    end
    add_packages(
        "borealis",
        "mpv",
        "cpr",
        "qr-code-generator",
        "lunasvg",
        "opencc",
        "pystring",
        "sdl2"
    )
    if is_plat("windows", "mingw") then
        add_files("app_win32.rc")
        after_build(function (target) 
            if get_config("winrt") then
                import("uwp")(target)
            else
                for _, pkg in pairs(target:pkgs()) do
                    if pkg:has_shared() then
                        for _, f in ipairs(pkg:libraryfiles()) do
                            if f:endswith(".dll") or f:endswith(".so") then
                                os.cp(f, target:targetdir().."/")
                            end
                        end
                    end
                end
                os.cp("resources", target:targetdir().."/")
            end
        end)
    end

target("demo/sw")
    add_packages(
        "mpv",
        "sdl2"
    )
    if is_plat("windows", "mingw") then
        add_files("app_win32.rc")
    end
    add_files("demo/sw.c")
