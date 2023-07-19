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
    local sourcedirs = {
        path.join(path.directory(os.projectfile()),"../borealis"),
        path.join(path.directory(os.projectfile()), "./build/xrepo/borealis"),
        path.join(path.directory(os.projectfile()), "./library/borealis"),
    }
    for _, sourcedir in ipairs(sourcedirs) do
        if os.exists(sourcedir) and os.isdir(sourcedir) then
            set_sourcedir(sourcedir)
            break
        end
    end
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

package("mongoose")
    set_urls("https://github.com/xfangfang/mongoose/archive/6cb500ca1e2cb2be36e5e2592547cd0803a6a90b.zip")

    add_versions("latest", "4b72b2aa0a18a4fb4161e60d6d1d16d2dd595f2b9b97a0fd9cd7fb80a68fb12f")
    
    on_install(function (package)
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            target("mongoose")
                set_kind("$(kind)")
                add_headerfiles("mongoose.h")
                add_files("mongoose.c")
        ]])
        local configs = {}
        configs["kind"] = package:config("shared") and "shared" or "static"
        import("package.tools.xmake").install(package, configs)
    end)
package_end()

package("pdr")
    local sourcedirs = {
        path.join(path.directory(os.projectfile()),"../libpdr"),
        path.join(path.directory(os.projectfile()), "./build/xrepo/libpdr"),
        path.join(path.directory(os.projectfile()), "./library/libpdr"),
    }
    for _, sourcedir in ipairs(sourcedirs) do
        if os.exists(sourcedir) and os.isdir(sourcedir) then
            set_sourcedir(sourcedir)
            break
        end
    end
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
    before_build(function (target)
        local GIT_TAG_VERSION = vformat("$(shell git describe --tags)")
        local GIT_TAG_SHORT = vformat("$(shell git rev-parse --short HEAD)")
        local cmakefile = io.readfile("CMakeLists.txt")
        local VERSION_MAJOR = string.match(cmakefile, "set%(VERSION_MAJOR \"(%d)\"%)")
        local VERSION_MINOR = string.match(cmakefile, "set%(VERSION_MINOR \"(%d)\"%)")
        local VERSION_REVISION = string.match(cmakefile, "set%(VERSION_REVISION \"(%d)\"%)")
        local PACKAGE_NAME = string.match(cmakefile, "set%(PACKAGE_NAME ([^%)]+)%)")
        target:add(
            "defines",
            "BUILD_TAG_VERSION="..GIT_TAG_VERSION,
            "DBUILD_TAG_SHORT="..GIT_TAG_SHORT,
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
        "webp",
        "mongoose",
        "zlib",
        "pdr"
    )
    if is_plat("windows", "mingw") then
        add_files("app_win32.rc")
        add_syslinks("Wlanapi", "iphlpapi")
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

    if is_plat("mingw") then
        add_ldflags("-static-libgcc", "-static-libstdc++")
    end
    if is_mode("release") then
        if is_plat("mingw") then
            add_cxflags("-Wl,--subsystem,windows", {force = true})
            add_ldflags("-Wl,--subsystem,windows", {force = true})
        elseif is_plat("windows") then
            add_ldflags("/SUBSYSTEM:WINDOWS")
        end
    end
