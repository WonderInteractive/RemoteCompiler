#include "helper.h"
#include <iostream>
#include "compressor.h"
#include <csignal>
#include <random>
void exiting() {
    //size_t result = boost::stacktrace::safe_dump_to("./backtrace.dump");
    //std::cout << boost::stacktrace::stacktrace() << std::endl;
    return;
}
void starting() {
    return;
}
std::chrono::time_point<std::chrono::high_resolution_clock> timer_start() { return std::chrono::high_resolution_clock::now(); }
double timer_end(std::chrono::time_point<std::chrono::high_resolution_clock> start) { std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start; return duration.count(); }
auto timer_fin(std::chrono::time_point<std::chrono::high_resolution_clock> start) { return std::chrono::high_resolution_clock::now() - start; }

void replace(std::string& input, const std::string& from, const std::string& to) {
    auto pos = 0;
    while (true)
    {
        size_t startPosition = input.find(from, pos);
        if (startPosition == std::string::npos)
            return;
        input.replace(startPosition, from.length(), to);
        pos += to.length();
    }
}

union u
{
    uint32_t num32;
    struct
    {
        int8_t a;
        int8_t b;
        int8_t c;
        int8_t d;

    } bytes;
} converter1;
u32 bytes_to_u32(const std::string& str) {
    converter1.bytes.a = str[0];
    converter1.bytes.b = str[1];
    converter1.bytes.c = str[2];
    converter1.bytes.d = str[3];
    return converter1.num32;
}
std::string u32_to_bytes(const u32 num) {
    converter1.num32 = num;
    return { converter1.bytes.a, converter1.bytes.b, converter1.bytes.c, converter1.bytes.d };
}
std::vector<std::string> split(std::string strToSplit, char delimeter) {
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter))
        splittedStrings.push_back(item);
    return splittedStrings;
}
fnhash GetHash(const std::string& fn) {
    //could always hash filename as it's probably not slow, otherwise can keep a hashmap of already done hashes to improve performance
    return 0;
}
std::string fnhash_to_filename(fnhash hash) {
    return "";
}
SetT<u32> waiting;
void SetWait(u8 id) {
    waiting.insert(id);
};
void Wait(u8 id, std::chrono::duration<double> wait_for) {
    SetWait(id);
    auto start = timer_start();
    while (waiting.contains(id)) {
        auto now = timer_fin(start);
        if (now > wait_for) {
            break;
        }
    }
}

void WaitAll(std::chrono::duration<double> wait_for) {
    auto start = timer_start();
    for (auto id : waiting) {
        Wait(id, wait_for);
        auto now = timer_fin(start);
        wait_for -= now;
    }
}

void UnWait(u8 id) {
    waiting.erase(id);
}
std::string bytesToStr(uint64_t bytes) {
    std::string postfix = "";
    double out = bytes;
    if (bytes > 1000000000000) {
        out = double(bytes) / 1000.0f / 1000.0f / 1000.0f / 1000.0f;
        postfix = "TB";
    }
    else if (bytes > 1000000000) {
        out = double(bytes) / 1000.0f / 1000.0f / 1000.0f;
        postfix = "GB";
    }
    else if (bytes > 1000000) {
        out = double(bytes) / 1000.0f / 1000.0f;
        postfix = "MB";
    }
    else if (bytes > 1000) {
        out = double(bytes) / 1000.0f;
        postfix = "KB";
    }
    else
        postfix = "B";
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << out;
    std::string s = stream.str();
    return s + postfix;
}

uint32_t genRandomId() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
    return dist(rd);
}

bool path_has_prefix(const fs::path& path, const fs::path& prefix) {
    auto pair = std::mismatch(path.begin(), path.end(), prefix.begin(), prefix.end());
    return pair.second == prefix.end();
}

uint16_t exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    uint16_t r = std::stoi(result);
    return r;
}

std::string exec2(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

