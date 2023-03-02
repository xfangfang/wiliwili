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

package("borealis")
    set_sourcedir("../borealis")
    add_deps(
        "nanovg",
        "yoga",
        "nlohmann_json",
        "glad",
        "fmt",
        "tweeny",
        "stb",
        "tinyxml2",
        "xfangfang_glfw"
    )
    add_includedirs("include")
    if is_plat("windows") then
        add_includedirs("include/compat")
    end
    on_install(function (package)
        local configs = {}
        import("package.tools.xmake").install(package, configs)
        os.cp("library/include/*", package:installdir("include").."/")
    end)
package_end()

package("mpv")
    if is_plat("windows", "mingw") then
        set_urls("https://github.com/zhongfly/mpv-winbuild/releases/download/2023-03-01/mpv-dev-x86_64-20230301-git-779d4f9.7z")
        add_versions("latest", "5b49e682548f40169dc52f8844344ad779d06f6656934753521e21b1e8e3e2d7")
    end
    add_links("mpv")
    on_install("windows", "mingw", function (package)
        import("detect.sdks.find_vstudio")
        os.cp("include/*", package:installdir("include").."/")
        os.cp("*.a", package:installdir("lib").."/")
        os.cp("*.dll", package:installdir("bin").."/")

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

add_requires("borealis")
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
    add_packages(
        "borealis",
        "mpv",
        "cpr",
        "qr-code-generator",
        "lunasvg",
        "opencc",
        "pystring"
    )
    if is_plat("windows", "mingw") then
        add_files("app_win32.rc")
        add_files("wiliwili/source/resource.manifest")
        after_build(function (target) 
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
        end)
    end
