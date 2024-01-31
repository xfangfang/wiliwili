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
    add_defines("NOMINMAX")
    set_languages("c++20")
    if is_mode("release") then
        set_optimize("faster")
    end
else
    set_languages("c++17")
end

-- add_repositories("zeromake https://github.com/zeromake/xrepo.git v0.1.0")
add_repositories("zeromake https://github.com/zeromake/xrepo.git")

package("borealis")
    set_sourcedir(path.join(os.projectdir(), "library/borealis"))

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
        add_syslinks("Wlanapi", "iphlpapi", "Ws2_32")
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
        configs["window"] = package:config("window")
        configs["driver"] = package:config("driver")
        configs["winrt"] = package:config("winrt") and "y" or "n"
        import("package.tools.xmake").install(package, configs)
        os.cp("library/include/*", package:installdir("include").."/")
        os.rm(package:installdir("include/borealis/extern"))
        os.cp("library/include/borealis/extern/libretro-common", package:installdir("include").."/")
    end)
package_end()

package("pdr")
    set_sourcedir(path.join(os.projectdir(), "library/libpdr"))
    add_deps("mongoose", "tinyxml2")
    on_install(function (package)
        io.writefile("xmake.lua", [[
add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_cxflags("/utf-8")
    set_languages("c++20")
    if is_mode("release") then
        set_optimize("faster")
    end
else
    set_languages("c++17")
end

add_requires("tinyxml2", "mongoose")
target("pdr")
    set_kind("$(kind)")
    add_includedirs("include")
    add_headerfiles("include/*.h")
    add_files("src/*.cpp")
    add_packages("tinyxml2", "mongoose")
]])
        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)
package_end()

package("mpv")
    if is_plat("windows", "mingw") then
        set_urls("https://github.com/zeromake/wiliwili/releases/download/v0.6.0/mpv-dev-x86_64-v3-20230514-git-9e716d6.7z")
        add_versions("20230514", "d56e3e10ea3f9362a0d9bb85ff3cd84f6e1fecfe66c87725040a82d5712b3f5f")
    end
    add_links("mpv")
    on_install("windows", "mingw", function (package)
        import("detect.sdks.find_vstudio")
        os.cp("include/*", package:installdir("include").."/")
        os.cp("*.a", package:installdir("lib").."/")
        os.cp("*.dll", package:installdir("bin").."/")
        if package:is_plat("windows") then
            -- 从 dll 里导出函数为 lib 文件，预编译自带 def 文件格式不正确，没法导出 lib
            if os.isfile("mpv.def") then
                local def_context = io.readfile("mpv.def")
                if not def_context:startswith("EXPORTS") then
                    io.writefile("mpv.def", format("EXPORTS\n%s", def_context))
                end
            end
            for _, vsinfo in pairs(find_vstudio()) do
                if vsinfo.vcvarsall then
                    os.setenv("PATH", vsinfo.vcvarsall[os.arch()]["PATH"])
                end
            end

            os.execv("lib.exe", {"/name:libmpv-2.dll", "/def:mpv.def", "/out:mpv.lib", "/MACHINE:X64"})
            os.cp("*.lib", package:installdir("lib").."/")
            os.cp("*.exp", package:installdir("lib").."/")
        end
    end)
package_end()

if get_config("winrt") then
    add_requires("sdl2", {configs={shared=true,winrt=true}})
    add_requireconfs("**.sdl2", {configs={shared=true,winrt=true}})
    add_requires("borealis", {configs={window="sdl",driver=get_config("driver"),winrt=true}})
    add_requireconfs("**.curl", {configs={winrt=true}})
else
    add_requires("borealis", {configs={window=get_config("window"),driver=get_config("driver")}})
end
add_requires("mpv", {configs={shared=true}})
add_requires("cpr")
add_requires("clip")
add_requires("lunasvg")
add_requires("opencc")
add_requires("pystring")
add_requires("qr-code-generator", {configs={cpp=true}})
add_requires("webp")
add_requires("mongoose")
add_requires("zlib")
add_requires("pdr")

target("wiliwili")
    add_includedirs("wiliwili/include", "wiliwili/include/api")
    add_files("wiliwili/source/**.cpp")
    add_defines("BRLS_RESOURCES=\"./resources/\"")
    on_config(function (target)
        local cmakefile = io.readfile("CMakeLists.txt")
        local VERSION_MAJOR = string.match(cmakefile, "set%(VERSION_MAJOR \"(%d)\"%)")
        local VERSION_MINOR = string.match(cmakefile, "set%(VERSION_MINOR \"(%d)\"%)")
        local VERSION_REVISION = string.match(cmakefile, "set%(VERSION_REVISION \"(%d)\"%)")
        local PACKAGE_NAME = string.match(cmakefile, "set%(PACKAGE_NAME ([^%)]+)%)")

        target:set("configvar", "VERSION_MAJOR", VERSION_MAJOR)
        target:set("configvar", "VERSION_MINOR", VERSION_MINOR)
        target:set("configvar", "VERSION_REVISION", VERSION_REVISION)
        target:set("configvar", "VERSION_BUILD", "$(shell git rev-list --count --all)")
        
        target:add(
            "defines",
            "BUILD_TAG_VERSION=$(shell git describe --tags)",
            "DBUILD_TAG_SHORT=$(shell git rev-parse --short HEAD)",
            "BUILD_VERSION_MAJOR="..VERSION_MAJOR,
            "BUILD_VERSION_MINOR="..VERSION_MINOR,
            "BUILD_VERSION_REVISION="..VERSION_REVISION,
            "BUILD_PACKAGE_NAME="..PACKAGE_NAME
        )
    end)
    local driver = get_config("driver")
    if driver == "opengl" then
        add_defines("BOREALIS_USE_OPENGL")
    elseif driver == "d3d11" then
        add_defines("BOREALIS_USE_D3D11")
    elseif driver == "metal" then
        add_defines("BOREALIS_USE_METAL")
    end
    add_defines("USE_WEBP")
    if get_config("window") == 'sdl' then
        add_defines("__SDL2__=1")
        add_packages("sdl2")
    else
        add_defines("__GLFW__=1")
    end
    if get_config("winrt") then
        add_defines("__WINRT__=1")
        add_configfiles("winrt/AppxManifest.xml.in")
    end
    if get_config("sw") then
        add_defines("MPV_SW_RENDER=1")
    end
    add_packages(
        "borealis",
        "mpv",
        "cpr",
        "clip",
        "qr-code-generator",
        "lunasvg",
        "opencc",
        "pystring",
        "webp",
        "mongoose",
        "zlib",
        "pdr"
    )
    if is_plat("windows", "mingw") then
        after_build(function (target) 
            if get_config("winrt") then
                import("uwp")(target)
            else
                for _, pkg in pairs(target:pkgs()) do
                    for _, f in ipairs(pkg:libraryfiles()) do
                        if f:endswith(".dll") or f:endswith(".so") then
                            os.cp(f, target:targetdir().."/")
                        end
                    end
                end
                os.cp("resources", target:targetdir().."/")
            end
        end)
    end

    if is_plat("mingw") then
        add_ldflags("-static-libgcc", "-static-libstdc++")
    end
    if is_mode("release") then
        if is_plat("mingw") then
            add_cxflags("-Wl,--subsystem,windows", {force = true})
            add_ldflags("-Wl,--subsystem,windows", {force = true})
        elseif is_plat("windows") then
            add_ldflags("/MANIFEST:EMBED", "/MANIFESTINPUT:wiliwili/source/resource.manifest", {force = true})
            add_ldflags("/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup", {force = true})
        end
    end
