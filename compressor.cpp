#include "compressor.h"

#if HAS_ZSTD
#define ZSTD_STATIC_LINKING_ONLY
extern "C" {
#include "zstd.c"
}
#endif

#if HAS_BROTLI
#include "brotli/encode.h"
#include "brotli/decode.h"
#endif

#if HAS_FLZMA2
#include "fast-lzma2.h"
#endif


#if HAS_BCM
std::string Compress(int level, const std::string& in);
std::string Decompress(const std::string& in);
#endif

#if HAS_LZ4
extern "C" {
#define LZ4_DISABLE_DEPRECATE_WARNINGS
#define LZ4_STATIC_LINKING_ONLY
#define LZ4F_STATIC_LINKING_ONLY
#define LZ4_HC_STATIC_LINKING_ONLY
#include "lz4frame.c"
#include "lz4.c"
#include "lz4hc.c"
//#include "lz4.h"
//#include "lz4hc.h"
#undef MINMATCH
}
#endif

#ifdef _WIN32
#include <windows.h>
unsigned long long getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}
#endif

#include "taskflow/taskflow.hpp"
extern tf::Executor executor;
extern tf::Taskflow taskflow;

CompressorNONE* Compressor::NONE = new CompressorNONE();

#if HAS_FLZMA2
CompressorFLZMA* Compressor::FLZMA2 = new CompressorFLZMA();
CompressorFLZMA::CompressorFLZMA() {
    name = "flzma";
    levels[0] = FL2_maxHighCLevel();
    levels[1] = FL2_maxCLevel();
    levels[2] = FL2_maxHighCLevel();
}
std::string CompressorFLZMA::Compress(const std::string& to_compress, int override_level) {
    std::string compressed; compressed.resize(to_compress.size() * 4);
    auto out_size = FL2_compress(compressed.data(), compressed.size(), to_compress.data(), to_compress.size(), override_level);
    compressed.resize(out_size);
    compressed.shrink_to_fit();
    return compressed;
};
void CompressorFLZMA::CompressMt(std::string& to_compress, int override_level) {
    std::string compressed; compressed.resize(to_compress.size() * 4);
    auto out_size = FL2_compressMt(compressed.data(), compressed.size(), to_compress.data(), to_compress.size(), override_level, 0);
    compressed.resize(out_size);
    compressed.shrink_to_fit();
    to_compress = std::move(compressed);
};
void CompressorFLZMA::Decompress(std::string& to_decompress, const int out_size) {
    std::string decompressed; decompressed.resize(out_size);
    auto out_size2 = FL2_decompress(decompressed.data(), decompressed.size(), to_decompress.data(), to_decompress.size());
    if (out_size2 != out_size) {
        fmt::print("Error decompressing FLZMA\n");
    }
    to_decompress = decompressed;
};
void CompressorFLZMA::DecompressMt(std::string& to_decompress, const int out_size) {
    std::string decompressed; decompressed.resize(out_size);
    auto out_size2 = FL2_decompressMt(decompressed.data(), decompressed.size(), to_decompress.data(), to_decompress.size(), 0);
    if (out_size2 != out_size) {
        fmt::print("Error decompressing FLZMA\n");
    }
    to_decompress = decompressed;
};
#endif

#if HAS_BROTLI
CompressorBROTLI* Compressor::Brotli = new CompressorBROTLI();
CompressorBROTLI::CompressorBROTLI() {
    name = "brotli";
    levels[0] = 9;
    levels[1] = 11;
    levels[2] = 11;
}
std::string CompressorBROTLI::Compress(const std::string& to_compress, int override_level) {
    size_t size = BrotliEncoderMaxCompressedSize(to_compress.size());
    std::string compressed; compressed.resize(size);
    bool success = BrotliEncoderCompress(override_level, 24, BROTLI_DEFAULT_MODE, to_compress.size(), (const uint8_t*)to_compress.data(), &size, (unsigned char*)compressed.data());
    compressed.resize(size);
    compressed.shrink_to_fit();
    return compressed;
}
//auto avail_mem = getTotalSystemMemory();
std::string CompressorBROTLI::CompressMt(const std::string& to_compress, int override_level) {
    auto num_parts = 1;
    std::vector<std::vector<uint8_t>> compressed(num_parts);
    std::string compressed2;
    auto l = [&compressed](int i, int num_parts, int override_level, const std::string& to_compress) {
        size_t part_start = (i * to_compress.size()) / num_parts;
        size_t part_end = ((i + 1) * to_compress.size()) / num_parts;
        auto& piece = compressed[i];
        std::vector<uint8_t> original_piece(
            to_compress.begin() + part_start, to_compress.begin() + part_end);
        size_t available_in = original_piece.size();
        size_t available_in2 = original_piece.size();
        const uint8_t* next_in = original_piece.data();
        const uint8_t* next_in2 = original_piece.data();

        piece.resize(available_in + 1024, 0);
        size_t available_out = piece.size();
        uint8_t* next_out = reinterpret_cast<uint8_t*>(&piece[0]);
        size_t available_out2 = piece.size();
        uint8_t* next_out2 = reinterpret_cast<uint8_t*>(&piece[0]);

        BrotliEncoderState* enc =
            BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_QUALITY, override_level);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_MODE, BROTLI_DEFAULT_MODE);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_LGWIN, BROTLI_LARGE_MAX_WINDOW_BITS);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_LARGE_WINDOW, BROTLI_TRUE);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_STREAM_OFFSET, part_start);
        BrotliEncoderSetParameter(enc, BROTLI_PARAM_SIZE_HINT, to_compress.size());
        auto op =
            (i == num_parts - 1) ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_FLUSH;
        bool compress_ok = true;
        bool has_more_output = true;
        size_t total_out = 0;
        size_t total_out2 = 0;
        //auto result = BrotliEncoderCompressStream(enc, BROTLI_OPERATION_FINISH,
        //    &available_in2, &next_in2, &available_out2, &next_out2, &total_out);
        while (compress_ok && (available_in > 0 || has_more_output)) {
            compress_ok = BrotliEncoderCompressStream(enc, op, &available_in,
                &next_in, &available_out, &next_out, &total_out2);
            has_more_output = BrotliEncoderHasMoreOutput(enc);
        }
        BrotliEncoderDestroyInstance(enc);
        piece.resize(total_out2);
    };
    if (0) {
        taskflow.for_each_index(0, num_parts, 1, [&](int i) {
            l(i, num_parts, override_level, to_compress);
            });
        executor.run(taskflow).get();
    } else
    for (size_t i = 0; i < num_parts; ++i) {
        l(i, num_parts, override_level, to_compress);
    }
    for (auto& part : compressed) {
        compressed2.insert(compressed2.end(),
            part.begin(), part.end());
    }
    return compressed2;
}
void CompressorBROTLI::Decompress(std::string& to_decompress, const u64 out_size) {
    size_t decompressed_size_guess = 4 * to_decompress.size(); // based on compression ratio from benchmarks

    BrotliDecoderState* state = BrotliDecoderCreateInstance(0, 0, 0);
    size_t available_in = to_decompress.size(); // bytes left in the input
    const uint8_t* next_in = (uint8_t*)to_decompress.data(); // pointer to next byte in the input
    std::string decompressedBuffer; decompressedBuffer.resize(decompressed_size_guess);
    size_t total_out = 0; // number of output bytes written
    BrotliDecoderResult result;

    do {

        size_t available_out = decompressedBuffer.size() - total_out; // remaining space in output buffer
        uint8_t* next_out = (uint8_t*)&decompressedBuffer[total_out]; // next place to write output
        result = BrotliDecoderDecompressStream(
            state,
            &available_in,
            &next_in,
            &available_out,
            &next_out,
            &total_out
        );

        if (result == BROTLI_DECODER_RESULT_SUCCESS) {
            decompressedBuffer.resize(total_out);
            break;
        }

        // grow the buffer
        decompressedBuffer.resize(2 * decompressedBuffer.size());

    } while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT);
    to_decompress = decompressedBuffer;
};
#endif

#ifdef HAS_ZSTD
CompressorZSTD* Compressor::ZSTD = new CompressorZSTD();
CompressorZSTD::CompressorZSTD() {
    name = "zstd";
    levels[0] = 1;
    levels[1] = 22;
    levels[2] = 22;
}
std::string CompressorZSTD::Compress(const std::string& to_compress, int override_level) {
    size_t const cBuffSize = ZSTD_compressBound(to_compress.size());
    std::string compressed; compressed.resize(cBuffSize);
#if 1
    size_t const cSize = ZSTD_compress((void*)compressed.data(), cBuffSize, (const void*)to_compress.data(), to_compress.size(), override_level);
#else
    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, override_level);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_strategy, ZSTD_btultra2);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_windowLog, 8);
    size_t const cSize = ZSTD_compress2(cctx, (void*)compressed.data(), cBuffSize, (const void*)to_compress.data(), to_compress.size());
#endif
    compressed.resize(cSize);
    compressed.shrink_to_fit();
    return compressed;
};
void CompressorZSTD::Decompress(std::string& to_decompress, const u64 out_size) {
    unsigned long long rSize = out_size;
    if (out_size == 0)
        rSize = ZSTD_getDecompressedSize(to_decompress.data(), to_decompress.size()) * 2;
    std::string uncompressed; uncompressed.resize(rSize);
    size_t const cSize = ZSTD_decompress((void*)uncompressed.data(), rSize, (const void*)to_decompress.data(), to_decompress.size());
    if (ZSTD_isError(cSize)) {
        //fmt::print("error decompressing\n");
    }
    //uncompressed.resize(cSize);
    to_decompress = uncompressed;
}
std::string CompressorZSTD::GenDiff(const std::string& from, const std::string& to) {
    ZSTD_CCtx* const cctx = ZSTD_createCCtx();
    ZSTD_CDict* const cdict = ZSTD_createCDict(from.data(), from.size(), ZSTD_maxCLevel());
    size_t const cBuffSize = ZSTD_compressBound(to.size());
    std::string compressed; compressed.resize(cBuffSize);
    //Compressor::ZSTD->Compress(buffer, ZSTD_maxCLevel());
    auto cSize = ZSTD_compress_usingCDict(cctx, compressed.data(), compressed.size(), to.data(), to.size(), cdict);
    if (ZSTD_isError(cSize)) {
        auto ec = ZSTD_getErrorCode(cSize);
        auto err_str = ZSTD_getErrorString(ec);
        //fmt::print("Error generating diff because of the following error: '{}'.\n", err_str);
        exit(0);
    }
    compressed.resize(cSize);
    compressed.shrink_to_fit();
    ZSTD_freeCDict(cdict);
    ZSTD_freeCCtx(cctx);
    return compressed;
}
std::string CompressorZSTD::ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) {
    std::string patched; patched.resize(new_size);
    ZSTD_DCtx* const dctx = ZSTD_createDCtx();
    //ZSTD_CDict* const cdict = ZSTD_createCDict(from.data(), from.size(), ZSTD_maxCLevel());
    //decompress original on client and set as 'from' then run the following, once done, recompress and store back in db
    ZSTD_decompress_usingDict(dctx, patched.data(), patched.size(), patch.data(), patch.size(), from.data(), from.size());
    ZSTD_freeDCtx(dctx);
    return patched;
}
#endif

#if HAS_BCM
CompressorBCM* Compressor::BCM = new CompressorBCM();
CompressorBCM::CompressorBCM() {
    name = "bcm";
    levels[0] = 9;
    levels[1] = 5;
    levels[2] = 9;
}
std::string CompressorBCM::Compress(const std::string& to_compress, int override_level) {
    return ::Compress(override_level, to_compress);
}

void CompressorBCM::Decompress(std::string& to_decompress, const int out_size) {
    to_decompress = ::Decompress(to_decompress);
}
#endif

#if HAS_LZ4
CompressorLZ4* Compressor::LZ4 = new CompressorLZ4();
CompressorLZ4::CompressorLZ4() {
    name = "lz4";
    levels[0] = 12;
    levels[1] = 9;
    levels[2] = 12;
}
std::string CompressorLZ4::Compress(const std::string& to_compress, int override_level) {
    const size_t cBuffSize = LZ4_compressBound(to_compress.size());
    std::string out; out.resize(cBuffSize);
    size_t const cSize = LZ4_compress_HC(to_compress.data(), out.data(), to_compress.size(), cBuffSize, override_level);
    out.resize(cSize);
    out.shrink_to_fit();
    return out;
}

void CompressorLZ4::Decompress(std::string& to_decompress, const u64 out_size) {
    std::string uncompressed; uncompressed.resize(out_size);
    size_t const cSize = LZ4_decompress_safe(reinterpret_cast<const char*>(to_decompress.data()), uncompressed.data(), to_decompress.size(), uncompressed.size());
    to_decompress = uncompressed;
}
#endif