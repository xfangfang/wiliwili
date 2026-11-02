/*
 * wiliwili - PS5 native title: link stub for the console's video decoder.
 *
 * The console's hardware video decoder (libSceVideodec2, system module 207)
 * is not a normal library: the title imports its functions, the system
 * provides them once the module is loaded with sceSysmoduleLoadModule(), and
 * the loader fills the import slots at that moment. This file only gives the
 * linker the symbols; the addresses in the built title are the ones the
 * console's loader writes.
 *
 * The same approach is used by the VLC for PS5 port (ZiZc3/VLC, GPL-2.0-or-
 * later), which is where the layout of the decoder's structures comes from.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

int sceVideodec2QueryComputeMemoryInfo(void *info)
{
    (void)info;
    return -1;
}

int sceVideodec2AllocateComputeQueue(const void *config, const void *memory, void **queue)
{
    (void)config;
    (void)memory;
    (void)queue;
    return -1;
}

int sceVideodec2ReleaseComputeQueue(void *queue)
{
    (void)queue;
    return -1;
}

int sceVideodec2QueryDecoderMemoryInfo(const void *config, void *memory)
{
    (void)config;
    (void)memory;
    return -1;
}

int sceVideodec2CreateDecoder(const void *config, const void *memory, void **decoder)
{
    (void)config;
    (void)memory;
    (void)decoder;
    return -1;
}

int sceVideodec2DeleteDecoder(void *decoder)
{
    (void)decoder;
    return -1;
}

int sceVideodec2Decode(void *decoder, const void *input, void *frame, void *output)
{
    (void)decoder;
    (void)input;
    (void)frame;
    (void)output;
    return -1;
}

int sceVideodec2Flush(void *decoder, void *frame, void *output)
{
    (void)decoder;
    (void)frame;
    (void)output;
    return -1;
}

int sceVideodec2Reset(void *decoder)
{
    (void)decoder;
    return -1;
}

int sceVideodec2GetPictureInfo(const void *output, void *first, void *second)
{
    (void)output;
    (void)first;
    (void)second;
    return -1;
}
