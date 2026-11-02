//
// Created by fang on 2026/10/20.
//
// The console's hardware video decoder.
//
// The chip decodes H.264 and HEVC into pictures that live in the graphics
// memory. The decoder lives in a system module that the application loads on
// startup, and it works on access units in Annex B (with start codes).
//
// The layout of the decoder's structures and the way the chip wants its
// memory come from the VLC for PS5 port (ZiZc3/VLC, GPL-2.0-or-later).
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace ps5decoder
{

/**
 * Makes the queue the console's hardware video decoder runs on. The console
 * refuses one made after the application has asked for full access, so this
 * has to run before that.
 */
extern "C" int wiliwili_ps5_decoder_prepare(void);

/**
 * The queue the console's hardware video decoder runs on. The decoder the
 * player uses (the one in FFmpeg) runs on the same queue: the console
 * refuses a second one.
 */
extern "C" void* wiliwili_ps5_compute_queue(void);

enum Codec
{
    CODEC_NONE = 0,
    CODEC_AVC  = 1,
    CODEC_HEVC = 974921,
};

/**
 * A picture the chip has decoded. The data is NV12 (8 bit) or P010 (10 bit)
 * in the order the chip writes it: the luma plane, then the chroma plane.
 */
struct Picture
{
    const uint8_t* data   = nullptr;
    int width            = 0;
    int height           = 0;
    int pitch            = 0;  ///< bytes per luma row
    bool tenBit          = false;
    uint64_t pts         = 0;
};

class Decoder
{
public:
    Decoder() = default;
    ~Decoder();

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;

    /**
     * Starts the console's decoder.
     *
     * @param codec   CODEC_AVC or CODEC_HEVC
     * @param profile the profile as the codec numbers it (HEVC: 1 is Main,
     *                2 is Main 10)
     * @param level   the level as the codec numbers it
     * @param width   the picture's width, 0 when it is not known yet
     * @param height  the picture's height, 0 when it is not known yet
     */
    bool open(Codec codec, int profile, int level, int width, int height);

    /**
     * Decodes one access unit in Annex B. When a picture comes out, it is
     * copied into the caller's buffer and described by picture.
     *
     * @param annexb the access unit, with start codes
     * @param size   its length
     * @param pts    the access unit's time
     * @param picture describes the picture that came out
     * @param buffer  room for the picture, NV12 or P010
     * @param bufferSize the room's size
     * @return true when a picture was produced
     */
    bool decode(const uint8_t* annexb, size_t size, uint64_t pts, Picture& picture, uint8_t* buffer,
                size_t bufferSize);

    /// Takes the pictures that are still inside the decoder.
    bool flush(Picture& picture, uint8_t* buffer, size_t bufferSize);

    void close();

    /// True when the console's decoder is there and working.
    static bool available();

    /**
     * Decodes the test streams that ship with the application and reports
     * the pictures that come out. The console's decoder gives up on work it
     * does not like, and that must not take the application with it.
     */
    static void selfTest();

private:
    static void selfTestInner();

public:

private:
    struct Impl;
    Impl* impl = nullptr;
};

}  // namespace ps5decoder
