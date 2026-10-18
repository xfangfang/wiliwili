async function main() {
    const CWD = window.workingDir;
    // the application is linked against libGL.so.1 of the pacbrew mesa, which
    // in turn loads libgallium-26.2.0.so and libvulkan.so.1 at runtime,
    // point the dynamic loader at the directories they are installed in
    const ENV = {LD_LIBRARY_PATH: '/user/homebrew/lib:/data/homebrew/lib:' + CWD};
    return {
        mainText: "wiliwili",
        secondaryText: "a third-party bilibili client",
        onclick: async () => {
            return {
                path: CWD + '/wiliwili.elf',
                cwd: CWD,
                env: ENV
            };
        }
    };
}
