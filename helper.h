#pragma once
#define UWS_NO_ZLIB
#define HAS_FLZMA2 0
#define HAS_BROTLI 1
#define HAS_ZSTD 1
#define HAS_BCM 0
#define HAS_LZ4 0
#define USE_WEBSOCKETS 1
#define MAX_STRING_WAIT_TIME 500000000

#include <filesystem>
#include <string>
#include <chrono>
#include <cmath>
#include <sstream>
#include <fstream>
#include "archive.h"
#include <chrono>
#include <cstdlib>
#include <map>
#include <future>

#ifdef _WIN32
#define pclose _pclose
#define popen _popen
#define chdir _chdir
#endif

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using i8 = int8_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

std::chrono::time_point<std::chrono::high_resolution_clock> timer_start();
double timer_end(std::chrono::time_point<std::chrono::high_resolution_clock> start);
std::vector<std::string> split(std::string strToSplit, char delimeter);

u32 bytes_to_u32(const std::string& str);
std::string u32_to_bytes(const u32 num);

enum struct CompressMode { NONE, FAST, MAX, EXTREME };
enum struct PackageMode { GAME, PROJECT, MAP };

void replace(std::string& input, const std::string& from, const std::string& to);

uint16_t exec(const char* cmd);
std::string exec2(const char* cmd);


std::string bytesToStr(uint64_t bytes);

bool path_has_prefix(const fs::path& path, const fs::path& prefix);

uint32_t genRandomId();

fnhash GetHash(const std::string& fn);
std::string fnhash_to_filename(fnhash hash);
