/*
 * wiliwili - PS5 native title: the console's hardware video decoder.
 *
 * The decoder lives in system module 207 (libSceVideodec2). A title imports
 * its functions, the console's loader writes the module's addresses into the
 * import slots once the module is loaded, and the calls use those addresses.
 *
 * The slots are checked before anything is called: an import that the loader
 * did not fill holds zero, and a call through it would jump to zero.
 *
 * The layout of the decoder's structures comes from the VLC for PS5 port
 * (ZiZc3/VLC, GPL-2.0-or-later).
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "videodec2.h"

#include <stdio.h>

extern const char __wiliwili_text_start[];

// The system module that holds the decoder.
#define SYSMODULE_VIDEODEC2 207

extern "C" int sceSysmoduleLoadModule(uint16_t id);

namespace ps5videodec
{

// The addresses of the decoder's functions, so that the linker creates the
// imports the console's loader fills in. The table itself is only a way to
// make the imports exist; the calls go through the import slots.
__attribute__((used)) static const void* const kFunctions[10] = {
    (const void*)&sceVideodec2QueryComputeMemoryInfo,
    (const void*)&sceVideodec2AllocateComputeQueue,
    (const void*)&sceVideodec2ReleaseComputeQueue,
    (const void*)&sceVideodec2QueryDecoderMemoryInfo,
    (const void*)&sceVideodec2CreateDecoder,
    (const void*)&sceVideodec2DeleteDecoder,
    (const void*)&sceVideodec2Decode,
    (const void*)&sceVideodec2Flush,
    (const void*)&sceVideodec2Reset,
    (const void*)&sceVideodec2GetPictureInfo,
};

// The image base, computed from the text segment: a title is a position
// independent image, and its imports are offsets from that base.
static uintptr_t imageBase()
{
    return reinterpret_cast<uintptr_t>(__wiliwili_text_start) - (uintptr_t)kTextOffset;
}

bool resolved()
{
    if (kTextOffset == UINT64_MAX)
        return false;

    const uintptr_t base = imageBase();
    for (int i = 0; i < 10; i++) {
        if (kImportSlots[i] == 0)
            return false;
        uint64_t address = *(volatile uint64_t*)(base + kImportSlots[i]);
        if (address == 0) {
            fprintf(stderr, "[wiliwili] video decoder: %s is unresolved\n", kImportNames[i]);
            return false;
        }
    }
    return true;
}

} // namespace ps5videodec

/**
 * Load the console's video decoder and report whether its functions are
 * there. The module has to be loaded before the application asks for full
 * access, so this runs from the startup constructor.
 */
extern "C" int wiliwili_videodec2_prepare(void)
{
    int rc = sceSysmoduleLoadModule(SYSMODULE_VIDEODEC2);
    fprintf(stderr, "[wiliwili] video decoder sysmodule %d: rc=0x%08x\n", SYSMODULE_VIDEODEC2, rc);
    bool available = rc == 0 && ps5videodec::resolved();
    fprintf(stderr, "[wiliwili] hardware video decoding: %s\n", available ? "available" : "not available");
    fflush(stderr);
    return available ? 1 : 0;
}
