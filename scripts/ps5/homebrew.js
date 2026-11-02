// Launcher entry for the homebrew launcher served by the websrv payload
// (http://<ps5>:8080/), see https://github.com/ps5-payload-dev/websrv
async function main() {
    const CWD = window.workingDir;
    // wiliwili is linked against libGL.so.1 of the pacbrew mesa, which pulls in
    // libgallium-26.2.0.so and libvulkan.so.1. The build output carries the three
    // files in the lib directory next to the executable, point the dynamic loader
    // at that directory and at the ones the pacbrew packages install into.
    const ENV = {
        LD_LIBRARY_PATH: CWD + '/lib:/data/homebrew/lib:/user/homebrew/lib'
    };
    return {
        mainText: "wiliwili",
        secondaryText: "a third-party bilibili client",
        onclick: async () => {
            return {
                path: CWD + '/eboot.elf',
                cwd: CWD,
                env: ENV
            };
        }
    };
}
