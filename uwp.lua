local outPath = "build/wiliwili.msix"
local keyPath = "winrt/key.pfx"
local priconfigPath = "build/priconfig.xml"
local fileMapPath = "build/main.map.txt"
local priPath = "build/resources.pri"

function findsdk()
    local find_vstudio = import("detect.sdks.find_vstudio")
    for _, vsinfo in pairs(find_vstudio()) do
        if vsinfo.vcvarsall then
            return vsinfo.vcvarsall[os.arch()]
        end
    end
end

function main(target)
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
                    table.insert(files, {f, path.filename(f)})
                end
            end
        end
    end
    
    for _, f in ipairs(os.files("winrt/Assets/**")) do
        table.insert(files, {f, string.sub(f, 7)})
    end
    for _, f in ipairs(os.files("resources/**")) do
        table.insert(files, {f, f})
    end
    local file = io.open(fileMapPath, "w")
    file:write([[
[ResourceMetadata]
"ResourceDimensions"	"language-en-US"
[Files]
]])
    for _, ff in ipairs(files) do
        file:write(format('"%s"\t\t"%s"\n', ff[1], ff[2]))
    end
    file:close()

    os.setenv("PATH", findsdk()["PATH"])

    os.vexecv("makepri", {
        "createconfig",
        "-Overwrite",
        "/cf",
        priconfigPath,
        "/dq",
        "en-US"
    })
    os.vexecv("makepri", {
        "new",
        "-Overwrite",
        "/pr",
        "winrt",
        "/cf",
        priconfigPath,
        "-OutputFile",
        priPath
    })
    os.vexecv("makeappx", {
        "pack",
        "/l",
        "/h",
        "SHA256",
        "/f",
        fileMapPath,
        "/m",
        "build/AppxManifest.xml",
        "/o",
        "/p",
        outPath
    })
    os.vexecv("signtool", {
        "sign",
        "/fd",
        "SHA256",
        "/a",
        "/f",
        keyPath,
        outPath
    })
end