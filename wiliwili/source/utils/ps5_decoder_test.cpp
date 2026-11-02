//
// Created by fang on 2026/10/20.
//
// A test of the console's hardware video decoder: the small streams that
// ship with the application are decoded and the pictures that come out are
// reported.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#ifdef PS5

#include "utils/ps5_decoder.hpp"
#include "utils/ps5_decoder_test.hpp"

#include <chrono>
#include <vector>

#include <borealis/core/logger.hpp>

namespace ps5decoder
{

namespace
{

/**
 * One network abstraction layer unit: the start code and the bytes after it.
 */
struct Nal
{
    const uint8_t* data = nullptr;
    size_t size         = 0;
    int type            = 0;
};

/**
 * Splits an Annex B stream into its units. The parameter sets are kept
 * apart: the chip wants them in front of the first picture.
 */
void splitAnnexB(const uint8_t* data, size_t size, std::vector<Nal>& nals, std::vector<Nal>& headers,
                 bool hevc)
{
    std::vector<size_t> starts;
    for (size_t i = 0; i + 3 < size; i++) {
        if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
            starts.push_back(i);
            i += 2;
        }
    }
    for (size_t i = 0; i < starts.size(); i++) {
        size_t begin = starts[i] + 3;
        size_t end   = (i + 1 < starts.size()) ? starts[i + 1] : size;
        Nal nal      = {};
        nal.data     = data + begin;
        nal.size     = end - begin;
        // H.264 keeps the type in the low five bits, HEVC in six bits of the
        // first two bytes.
        nal.type = (nal.size > 0) ? (hevc ? ((nal.data[0] >> 1) & 0x3f) : (nal.data[0] & 0x1f)) : 0;
        if (nal.type == 7 || nal.type == 8) {
            // H.264: the sequence and picture parameter sets.
            headers.push_back(nal);
        } else if (nal.type == 32 || nal.type == 33 || nal.type == 34) {
            // HEVC: the video, sequence and picture parameter sets.
            headers.push_back(nal);
        } else {
            nals.push_back(nal);
        }
    }
}

/**
 * Decodes one stream and reports the pictures that came out.
 */
void runTest(const char* name, const unsigned char* data, unsigned int size, Codec codec, int profile,
             int level, int width, int height)
{
    if (!Decoder::available()) {
        brls::Logger::error("ps5 decoder: {}: the console's decoder is not there", name);
        return;
    }

    Decoder decoder;
    if (!decoder.open(codec, profile, level, width, height)) {
        brls::Logger::error("ps5 decoder: {}: the decoder did not start", name);
        return;
    }

    std::vector<Nal> nals;
    std::vector<Nal> headers;
    splitAnnexB(data, size, nals, headers, codec == CODEC_HEVC);
    brls::Logger::info("ps5 decoder: {}: {} units, {} parameter sets", name, nals.size(), headers.size());

    // The chip wants one access unit per picture: a picture is a slice and
    // whatever comes with it, the parameter sets of the first picture and the
    // like.
    const bool hevc = codec == CODEC_HEVC;
    std::vector<std::vector<Nal>> pictures;
    bool hasSlice = false;
    for (const Nal& nal : nals) {
        bool slice = hevc ? (nal.type <= 31) : (nal.type == 1 || nal.type == 5);
        if (slice && hasSlice) {
            // A second slice: the picture before it is complete.
            pictures.emplace_back();
            hasSlice = false;
        }
        if (pictures.empty())
            pictures.emplace_back();
        pictures.back().push_back(nal);
        if (slice)
            hasSlice = true;
    }

    brls::Logger::info("ps5 decoder: {}: {} pictures", name, pictures.size());

    // The parameter sets go in front of every picture: the chip wants them
    // with the slice they belong to.
    std::vector<uint8_t> accessUnit;
    accessUnit.reserve(1 << 16);
    int shown = 0;
    int sent  = 0;
    auto started = std::chrono::steady_clock::now();

    for (const std::vector<Nal>& picture : pictures) {
        accessUnit.clear();
        // The parameter sets go in front of every picture: the chip wants
        // them with the slice they belong to.
        for (const Nal& header : headers) {
            accessUnit.push_back(0);
            accessUnit.push_back(0);
            accessUnit.push_back(0);
            accessUnit.push_back(1);
            accessUnit.insert(accessUnit.end(), header.data, header.data + header.size);
        }
        for (const Nal& nal : picture) {
            accessUnit.push_back(0);
            accessUnit.push_back(0);
            accessUnit.push_back(0);
            accessUnit.push_back(1);
            accessUnit.insert(accessUnit.end(), nal.data, nal.data + nal.size);
        }

        std::vector<uint8_t> buffer(1920 * 1080 * 3);
        Picture out = {};
        if (decoder.decode(accessUnit.data(), accessUnit.size(), sent, out, buffer.data(), buffer.size())) {
            shown++;
            brls::Logger::info("ps5 decoder: {}: picture {} is {}x{}, {} bit", name, shown, out.width, out.height,
                               out.tenBit ? 10 : 8);
        }
        // The chip buffers the picture: it comes out with the next call.
        if (decoder.flush(out, buffer.data(), buffer.size())) {
            shown++;
            brls::Logger::info("ps5 decoder: {}: picture {} is {}x{}, {} bit", name, shown, out.width, out.height,
                               out.tenBit ? 10 : 8);
        }
        sent++;
    }

    // The decoder holds its pictures: they come out when it is drained.
    for (int i = 0; i < 32; i++) {
        std::vector<uint8_t> picture(1920 * 1080 * 3);
        Picture out = {};
        if (!decoder.flush(out, picture.data(), picture.size()))
            break;
        shown++;
        brls::Logger::info("ps5 decoder: {}: drained picture {} is {}x{}, {} bit", name, shown, out.width,
                           out.height, out.tenBit ? 10 : 8);
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - started)
                       .count();
    brls::Logger::info("ps5 decoder: {}: {} pictures in {} ms", name, shown, elapsed);
}

}  // namespace

void Decoder::selfTest()
{
    ps5decoder::runWithGuard([]() { Decoder::selfTestInner(); });
}

void Decoder::selfTestInner()
{
    brls::Logger::info("ps5 decoder: the console's decoder is {}", Decoder::available() ? "there" : "not there");
    // The levels the console takes: H.264 counts by ten, HEVC by thirty.
    // The profile and the level of HEVC go with the size of the picture:
    // 4.1 for 1080p, 5.0 for 1440p and 5.1 for 4K.
    runTest("H.264 baseline", ps5TestH264Baseline, ps5TestH264BaselineSize, CODEC_AVC, 66, 51, 320, 180);
    runTest("H.264 main", ps5TestH264Main, ps5TestH264MainSize, CODEC_AVC, 77, 51, 320, 180);
    // The console wants room for 1080p for HEVC, even for a small picture.
    runTest("HEVC main", ps5TestHevcMain, ps5TestHevcMainSize, CODEC_HEVC, 1, 123, 1920, 1088);
    runTest("HEVC main (Main 10)", ps5TestHevcMain, ps5TestHevcMainSize, CODEC_HEVC, 2, 123, 1920, 1088);
}

}  // namespace ps5decoder

#endif  // PS5
