import("core.base.option")

local resizes = {
    {"50x50", "StoreLogo.png"},
    {"48x48", "LockScreenLogo.png"},
    {"88x88", "Square44x44Logo.png"},
    -- {"24x24", "Square44x44Logo.targetsize-24_altform-unplated.png"},
    {"300x300", "Square150x150Logo.png"},
    {"600x600", "SplashScreen.png", "1240x600"},
    {"300x300", "Wide310x150Logo.png","620x300"},
}

function main(...)
    -- local argv = option.parse({...}, options, "Test all the given or changed packages.")
    if not option.get("verbose") then
        option.save("main")
        option.set("verbose", true)
    end
    -- wiliwili.png: rsvg-convert --width=1024 --height=1024 resources/svg/cn.xfangfang.wiliwili.svg > wiliwili.png
    local wiliwili_png = path.join("build", "wiliwili.png")
    if not os.exists(wiliwili_png) then
        os.vexecv("rsvg-convert", {"--width=1024", "--height=1024", "resources/svg/cn.xfangfang.wiliwili.svg", "-o", wiliwili_png})
    end

    for _, resize in ipairs(resizes) do
        local out = path.join("winrt/Assets", resize[2])
        local argv = {"convert", "-resize", resize[1], wiliwili_png, out}
        os.vexecv("magick", argv)
        if #resize > 2 then
            os.vexecv("magick", {"convert", "-background", "none", "-gravity", "center", "-extent", resize[3], out, "build/tmp.png"})
            os.cp("build/tmp.png", out)
        end
    end
end
