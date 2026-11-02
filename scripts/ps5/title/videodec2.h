/*
 * wiliwili - PS5 native title: the console's hardware video decoder.
 *
 * The decoder lives in system module 207 (libSceVideodec2). A title imports
 * its functions, the console's loader writes the module's addresses into the
 * import slots once the module is loaded, and the addresses are what the
 * calls below use.
 *
 * The slots are checked before anything is called: an import that the loader
 * did not fill holds zero, and a call through it would jump to zero.
 *
 * The layout of the decoder's structures comes from the VLC for PS5 port
 * (ZiZc3/VLC, GPL-2.0-or-later).
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ps5videodec
{

// The decoder's functions, as the console's loader sees them.
extern "C" {
int sceVideodec2QueryComputeMemoryInfo(void* info);
int sceVideodec2AllocateComputeQueue(const void* config, const void* memory, void** queue);
int sceVideodec2ReleaseComputeQueue(void* queue);
int sceVideodec2QueryDecoderMemoryInfo(const void* config, void* memory);
int sceVideodec2CreateDecoder(const void* config, const void* memory, void** decoder);
int sceVideodec2DeleteDecoder(void* decoder);
int sceVideodec2Decode(void* decoder, const void* input, void* frame, void* output);
int sceVideodec2Flush(void* decoder, void* frame, void* output);
int sceVideodec2Reset(void* decoder);
int sceVideodec2GetPictureInfo(const void* output, void* first, void* second);
}

// The slots the loader writes, as offsets from the image base. The build
// fills them in from the linker's relocation table.
extern const uint64_t kImportSlots[10];
extern const char* const kImportNames[10];

// The address of the text segment in the image: the base of the image is the
// address of the text at run time minus this.
extern const uint64_t kTextOffset;

// True when the console's loader has written the module's addresses.
bool resolved();

} // namespace ps5videodec
