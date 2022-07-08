#pragma once
#include "helper.h"
struct Result {
    std::string name = "none";
    float ctime = 0.0;
    float dtime = 0.0;
    float ratio = 0.0;
    uint64_t size = 0;
    uint64_t csize = 0;
    std::string out = "";
};
struct Compressor {
    static struct CompressorFLZMA* FLZMA2;
    static struct CompressorBROTLI* Brotli;
    static struct CompressorZSTD* ZSTD;
    static struct CompressorBCM* BCM;
    static struct CompressorLZ4* LZ4;
    static struct CompressorNONE* NONE;
    std::string name = "";
    CompressMode level = CompressMode::FAST;
    int levels[3] = { 1,1,1 };
    void Compress(std::string& to_compress) {
        if (level == CompressMode::NONE)
            return;
        Compress(to_compress, levels[(int)level - 1]);
    };
    /* Test(const std::string& to_compress, CompressMode mode) {
        if (level == CompressMode::NONE)
            return {};
        Result out;
        std::string compressed = to_compress;
        out.size = to_compress.size();
        auto start = timer_start();
        Compress(compressed, levels[(int)mode - 1]);
        out.ctime = timer_end(start);
        out.csize = compressed.size();
        out.ratio = double(compressed.size()) / double(to_compress.size());
        insize = to_compress.size();
        start = timer_start();
        Decompress(compressed, insize);
        out.dtime = timer_end(start);
        out.name = name;
        if (compressed != to_compress) {
            fmt::print("error decompressing {}\n", name);
        }
        out.out = std::move(compressed);
        return out;
    };*/
    virtual std::string Compress(const std::string& to_compress, int override_level) = 0;
    virtual std::string CompressMt(const std::string& to_compress, int override_level) { return ""; };
    virtual void Decompress(std::string& to_compress, const u64 out_size) = 0;
    virtual void DecompressMt(std::string& to_compress, const u64 out_size) { return; };
    virtual std::string GenDiff(const std::string& from, const std::string& to) = 0;
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) = 0;
};
#if HAS_FLZMA
struct CompressorFLZMA : Compressor {
    CompressorFLZMA();
    virtual std::string Compress(const std::string& to_compress, int override_level);
    virtual std::string CompressMt(const std::string& to_compress, int override_level);
    virtual void Decompress(std::string& to_decompress, const u64 out_size);
    virtual void DecompressMt(std::string& to_decompress, const u64 out_size);
    virtual std::string GenDiff(const std::string& from, const std::string& to) { return ""; };
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) { return ""; };
};
#endif
#if HAS_BROTLI
struct CompressorBROTLI : Compressor {
    CompressorBROTLI();
    virtual std::string Compress(const std::string& to_compress, int override_level);
    virtual std::string CompressMt(const std::string& to_compress, int override_level);
    virtual void Decompress(std::string& to_decompress, const u64 out_size);
    virtual std::string GenDiff(const std::string& from, const std::string& to) { return ""; };
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) { return ""; };
};
#endif
struct CompressorBCM : Compressor {
    CompressorBCM();
    virtual std::string Compress(const std::string& to_compress, int override_level);
    virtual void Decompress(std::string& to_decompress, const u64 out_size);
    virtual std::string GenDiff(const std::string& from, const std::string& to) { return ""; };
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) { return ""; };
};
struct CompressorLZ4 : Compressor {
    CompressorLZ4();
    virtual std::string Compress(const std::string& to_compress, int override_level);
    virtual void Decompress(std::string& to_decompress, const u64 out_size);
    virtual std::string GenDiff(const std::string& from, const std::string& to) { return ""; };
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) { return ""; };
};
struct CompressorZSTD : Compressor {
    CompressorZSTD();
    virtual std::string Compress(const std::string& to_compress, int override_level);
    virtual void Decompress(std::string& to_decompress, const u64 out_size);
    virtual std::string GenDiff(const std::string& from, const std::string& to);
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size);
};
struct CompressorNONE : Compressor {
    CompressorNONE() {
        name = "none";
        levels[0] = 1;
        levels[1] = 22;
        levels[2] = 22;
    }
    virtual std::string Compress(const std::string& to_compress, int override_level) { return ""; };
    virtual void Decompress(std::string& to_decompress, const u64 out_size) {};
    virtual std::string GenDiff(const std::string& from, const std::string& to) { return ""; };
    virtual std::string ApplyDiff(const std::string& from, const std::string& patch, const uint32_t new_size) { return ""; };
};