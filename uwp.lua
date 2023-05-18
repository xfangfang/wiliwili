import("lib.detect.find_program")
import("detect.sdks.find_vstudio")
import("core.base.option")

local function listFiles(dir, out, i)
    for _, f in ipairs(os.filedirs(path.join(dir, "**"))) do
        if os.isfile(f) then
            t = f
            if i then
                t = string.sub(f, i)
            end
            table.insert(out, {f, t})
        end
    end
end

local outPath = "build/wiliwili.msix"
local keyPath = "winrt/key.pfx"
local priconfigPath = "build/priconfig.xml"
local fileMapPath = "build/main.map.txt"
local priPath = "build\\resources.pri"

function main(target)
    option.save("main")
    option.set("verbose", true)
    local files = {
        {priPath, "resources.pri"},
        {target:targetfile(), "wiliwili.exe"},
    }
    local d = {}
    local debugs = {
        path.join(target:targetdir(), "wiliwili.pdb"),
        path.join(target:targetdir(), "wiliwili.ilk"),
    }
    for _, f in ipairs(debugs) do
        if os.exists(f) then
            local k = path.filename(f)
            table.insert(files, {f, k})
        end
    end
    for _, pkg in pairs(target:pkgs()) do
        if pkg:has_shared() then
            for _, f in ipairs(pkg:libraryfiles()) do
                if f:endswith(".dll") then
                    local k = path.filename(f)
                    if d[k] == nil then
                        table.insert(files, {f, k})
                        d[k] = true
                    end
                end
            end
        end
    end
    listFiles("winrt/Assets", files, 7)
    listFiles("resources", files)
    local context = ""
    for _, ff in ipairs(files) do
        context = context..format('\n"%s"\t\t"%s"', ff[1], ff[2])
    end

    io.writefile(fileMapPath, format([[
[ResourceMetadata]
"ResourceDimensions"	"language-en-US"
[Files]
%s
]], context))
    local cmakefile = io.readfile("CMakeLists.txt")
    local VERSION_MAJOR = string.match(cmakefile, "set%(VERSION_MAJOR \"(%d)\"%)")
    local VERSION_MINOR = string.match(cmakefile, "set%(VERSION_MINOR \"(%d)\"%)")
    local VERSION_REVISION = string.match(cmakefile, "set%(VERSION_REVISION \"(%d)\"%)")

    local appxManifest = io.readfile("winrt/AppxManifest.xml.in")
    local VERSION_BUILD = tostring(os.time()):sub(6)
    local VERSION = string.format("%s.%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD)
    appxManifest = appxManifest:gsub("%$%(VERISON%)", VERSION)
    io.writefile("winrt/AppxManifest.xml", appxManifest)

    local vs = find_vstudio()["2022"]["vcvarsall"]["x86"]
    local windowsSdkBinPath = path.join(vs["WindowsSdkBinPath"], vs["WindowsSDKVersion"], "x86")
    local makepri = path.join(windowsSdkBinPath, "makepri.exe")
    local makeappx = path.join(windowsSdkBinPath, "makeappx.exe")
    local signtool = path.join(windowsSdkBinPath, "signtool.exe")

    os.vexecv(makepri, {
        "createconfig",
        "-Overwrite",
        "/cf",
        priconfigPath,
        "/dq",
        "en-US"
    })
    os.vexecv(makepri, {
        "new",
        "-Overwrite",
        "/pr",
        "winrt",
        "/cf",
        priconfigPath,
        "-OutputFile",
        priPath
    })
    os.vexecv(makeappx, {
        "pack",
        "/l",
        "/h",
        "SHA256",
        "/f",
        fileMapPath,
        "/m",
        "winrt/AppxManifest.xml",
        "/o",
        "/p",
        outPath
    })
    os.vexecv(signtool, {
        "sign",
        "/fd",
        "SHA256",
        "/a",
        "/f",
        keyPath,
        outPath
    })
    print(format("build msix: %s", VERSION))
end