#include "../simplecpp2.cpp"
#include <vector>
#include <string>
#include <iostream>
#include <taskflow/taskflow.hpp>
#include <cpr/cpr.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "fmt/format.h"
#include <direct.h>
#include "../compressor.h"
#include "../helper.h"
#include "../ipc.h"
#define TCPP_IMPLEMENTATION
#include "../tcpplibrary.hpp"

#ifndef _MACARON_BASE64_H_
#define _MACARON_BASE64_H_

#include <string>
#include <clang-c/Index.h>
using namespace tcpp;

namespace macaron {

    class Base64 {
    public:

        static std::string Encode(const std::string data) {
            static constexpr char sEncodingTable[] = {
              'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
              'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
              'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
              'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
              'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
              'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
              'w', 'x', 'y', 'z', '0', '1', '2', '3',
              '4', '5', '6', '7', '8', '9', '+', '/'
            };

            size_t in_len = data.size();
            size_t out_len = 4 * ((in_len + 2) / 3);
            std::string ret(out_len, '\0');
            size_t i;
            char* p = const_cast<char*>(ret.c_str());

            for (i = 0; i < in_len - 2; i += 3) {
                *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
                *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
                *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
                *p++ = sEncodingTable[data[i + 2] & 0x3F];
            }
            if (i < in_len) {
                *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
                if (i == (in_len - 1)) {
                    *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
                    *p++ = '=';
                }
                else {
                    *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
                    *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
                }
                *p++ = '=';
            }

            return ret;
        }

        static std::string Decode(const std::string& input, std::string& out) {
            static constexpr unsigned char kDecodingTable[] = {
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
              52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
              64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
              15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
              64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
              41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
              64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
            };

            size_t in_len = input.size();
            if (in_len % 4 != 0) return "Input data size is not a multiple of 4";

            size_t out_len = in_len / 4 * 3;
            if (input[in_len - 1] == '=') out_len--;
            if (input[in_len - 2] == '=') out_len--;

            out.resize(out_len);

            for (size_t i = 0, j = 0; i < in_len;) {
                uint32_t a = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
                uint32_t b = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
                uint32_t c = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];
                uint32_t d = input[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(input[i++])];

                uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

                if (j < out_len) out[j++] = (triple >> 2 * 8) & 0xFF;
                if (j < out_len) out[j++] = (triple >> 1 * 8) & 0xFF;
                if (j < out_len) out[j++] = (triple >> 0 * 8) & 0xFF;
            }

            return "";
        }

    };

}

#endif /* _MACARON_BASE64_H_ */

using namespace cpr;
tf::Executor executor;
tf::Taskflow taskflow;

//method to read a file
std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

//method to write a new file
void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::binary);
    file << content;
}
#define DOING_LOCAL 0
#define DOING_VERIFY 0
#define PROFILING 0
std::string profileOut;
void profile(std::string_view what, double time) {
#if PROFILING == 1
    profileOut += fmt::format("{} : {} ", what, time);
#elif PROFILING == 2
    fmt::print("{} : {}\n", what, time);
#endif
}
const std::string profileDir = "c://test//profile//";
//generate random number
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<u64> dis(0, UINT64_MAX);
u64 random_number = dis(gen);
auto temp_filename = fmt::format("{}", random_number);
std::string temp_filename2 = "";
void startProfile(float content, const std::string& filename) {
    temp_filename2 = profileDir + filename + temp_filename;
    std::ofstream file(temp_filename2, std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<char*>(&content), 4);
}
void appendProfile(float content) {
    std::ofstream file(temp_filename2, std::ios::out | std::ios::binary | std::ios_base::app);
    file.write(reinterpret_cast<char*>(&content), 4);
}
struct Dep {
    fs::path file;
    fs::path path;
    std::string include;
    std::string hash_path;
    u64 hash = 0;
    int64_t start = 0, end = 0;
    bool checked_deps = false;
    bool checked_path = false;
};
std::vector<Dep> getIncludes(std::string_view cpp, std::string_view file) {
    std::vector<Dep> out;
    int64_t _pos = 0;
    while (1) {
        int64_t found = cpp.find("#include");
        if (found < 0) break;
        int64_t found_0 = cpp.find('\"', found);
        int64_t found_1 = cpp.find('<', found);
        if (found_0 == -1 && found_1 == -1) {
            fmt::print("parsing error, expected a quote or angle bracket\n");
        }
        if (found_0 == -1) found_0 = INT64_MAX;
        if (found_1 == -1) found_1 = INT64_MAX;
        if (found_0 < found_1) {
            if (found_0 >= 0) {
                auto cpp2 = cpp.substr(found_0 + 1);
                int found_00 = cpp2.find('\"');
                if (found_00 >= 0) {
                    _pos += 1 + found_0;
                    out.emplace_back(Dep{ file, "", std::string(cpp2.substr(0, found_00)), "", 0, _pos, _pos + found_00 - 1 });
                    _pos += found_00 + 1;
                    cpp = cpp2.substr(found_00 + 1);
                }
                else {
                    fmt::print("parsing error, expected a closing quote\n");
                }
            }
        }
        if (found_1 < found_0) {
            if (found_1 >= 0) {
                auto cpp2 = cpp.substr(found_1 + 1);
                int found_11 = cpp2.find('>');
                if (found_11 >= 0) {
                    _pos += 1 + found_1;
                    out.emplace_back(Dep{ file, "", std::string(cpp2.substr(0, found_11)), "", 0, _pos, _pos + found_11 - 1 });
                    _pos += found_11 + 1;
                    cpp = cpp2.substr(found_11 + 1);
                }
                else {
                    fmt::print("parsing error, expected a closing angle bracket\n");
                }
            }
        }
    }
    return out;
}
void getPath(const std::vector<fs::path>& dirs, Dep& i) {
    if (i.include.size() == 0) return;
    for (auto& dir : dirs) {
        if (fs::exists(dir / i.include)) {
            i.path = dir / i.include;
            break;
        }
    }
}
std::vector<Dep> checkDeps(const std::vector<fs::path>& dirs, std::vector<Dep>& includes) {
    std::vector<Dep> out;
    for (auto& i : includes) {
        if (!i.checked_path) {
            getPath(dirs, i);
            i.checked_path = true;
        }
        if (!i.checked_deps && !i.path.empty() && i.start != 0) {
            auto o = getIncludes(readFile(i.path.generic_string()), i.path.generic_string());
            i.checked_deps = true;
            if (o.size() > 0) {
                using std::begin, std::end;
                out.insert(end(out), begin(o), end(o));
            }
        }
    }
    return out;
}
int simplecpp2(int argc, char** argv)
{
    std::string filename = "";


    simplecpp::DUI dui;
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (*arg == '-') {
            char c = arg[1];
            if (c != 'D' && c != 'U' && c != 'I' && c != 'i' && c != 's')
                continue;  // Ignored
            const char* value = arg[2] ? (argv[i] + 2) : argv[++i];
            switch (c) {
            case 'D': // define symbol
                dui.defines.push_back(value);
                break;
            case 'U': // undefine symbol
                dui.undefined.insert(value);
                break;
            case 'I': // include path
                dui.includePaths.push_back(value);
                break;
            case 'i':
                if (std::strncmp(arg, "-include=", 9) == 0)
                    dui.includes.push_back(arg + 9);
                break;
            case 's':
                if (std::strncmp(arg, "-std=", 5) == 0)
                    dui.std = arg + 5;
                break;
            }
        }
        else {
            filename = arg;
        }
    }
    dui.std = "c++17";
    auto parseResp = [&dui](const std::string& filename) {
        std::string filename2 = "";
        auto read = readFile(std::string(filename).substr(1));
        std::vector<std::string> _split = split(read, '\n');

        // Settings..
        for (int i = 0; i < _split.size(); i++) {
            while (_split[i].front() == ' ') _split[i] = _split[i].substr(1);
            std::string& arg = _split[i];
            if (arg.back() == '\r') arg = arg.substr(0, arg.size() - 1);
            if (arg[0] == '-' || arg[0] == '/') {
                char c = arg[1];
                if (c != 'D' && c != 'U' && c != 'I' && c != 'i' && c != 's' && c != 'F')
                    continue;  // Ignored
                std::string value = arg[2] ? (_split[i].data() + 2) : _split[++i].data();
                switch (c) {
                case 'D': // define symbol
                    dui.defines.push_back(value);
                    break;
                case 'U': // undefine symbol
                    dui.undefined.insert(value);
                    break;
                case 'I': // include path
                    if (value.back() == '\"') value = value.substr(0, value.size() - 1);
                    if (value.front() == '\"') value = value.substr(1);
                    while (value.front() == ' ') value = value.substr(1);
                    dui.includePaths.push_back(value);
                    break;
                case 'F':
                    if (arg.starts_with("/FI")) {
                        auto ii = arg.substr(3);
                        if (ii.back() == '\"') ii = ii.substr(0, ii.size() - 1);
                        if (ii.front() == '\"') ii = ii.substr(1);
                        dui.includes.push_back(ii);
                    }
                    break;
                case 'i':
                    if (arg.starts_with("-include="))
                        dui.includes.push_back(arg.substr(9));
                    if (arg.starts_with("/imsvc")) {
                        auto ii = arg.substr(6);
                        while (ii.front() == ' ') ii = ii.substr(1);
                        if (ii.back() == '\"') ii = ii.substr(0, ii.size() - 1);
                        if (ii.front() == '\"') ii = ii.substr(1);
                        dui.includePaths.push_back(ii);
                    }
                    
                    break;
                case 's':
                    if (arg.starts_with("-std="))
                        dui.std = arg.substr(7);
                    break;
                }
            }
            else {
                filename2 = std::string(arg);
                if (filename2.back() == '\r') filename2 = filename2.substr(0, filename2.size() - 1);
            }
        }
        return filename2;
    };

    dui.defines.push_back("__has_include(n)=1");


    //dui.defines.push_back("_M_X64");
    if (filename.size() == 0) {
        std::cout << "Syntax:" << std::endl;
        std::cout << "simplecpp [options] filename" << std::endl;
        std::cout << "  -DNAME          Define NAME." << std::endl;
        std::cout << "  -IPATH          Include path." << std::endl;
        std::cout << "  -include=FILE   Include FILE." << std::endl;
        std::cout << "  -UNAME          Undefine NAME." << std::endl;
        std::exit(0);
    }

    if (filename[0] == '@') {
        filename = parseResp(filename);
        if (filename.size() == 0) {
            std::cout << "Syntax:" << std::endl;
            std::cout << "simplecpp [options] filename" << std::endl;
            std::cout << "  -DNAME          Define NAME." << std::endl;
            std::cout << "  -IPATH          Include path." << std::endl;
            std::cout << "  -include=FILE   Include FILE." << std::endl;
            std::cout << "  -UNAME          Undefine NAME." << std::endl;
            std::exit(0);
        }
    }

    // Perform preprocessing
    
    simplecpp::OutputList outputList;
    std::vector<std::string> files;
    std::ifstream f(filename);
    simplecpp::TokenList rawtokens(f, files, filename, &outputList);
    rawtokens.removeComments();
    auto start = timer_start();
    std::map<std::string, simplecpp::TokenList*> included = simplecpp::load(rawtokens, files, dui, &outputList);
    for (std::pair<std::string, simplecpp::TokenList*> i : included)
        i.second->removeComments();
    simplecpp::TokenList outputTokens(files);
    simplecpp::preprocess(outputTokens, rawtokens, files, included, dui, &outputList);
    auto end = (float)timer_end(start);
    std::cout << "test" << end << std::endl;

    // Output
    //std::cout << outputTokens.stringify() << std::endl;
    for (const simplecpp::Output& output : outputList) {
        std::cerr << output.location.file() << ':' << output.location.line << ": ";
        switch (output.type) {
        case 0:
            std::cerr << "#error: ";
            break;
        case simplecpp::Output::WARNING:
            std::cerr << "#warning: ";
            break;
        case simplecpp::Output::MISSING_HEADER:
            std::cerr << "missing header: ";
            break;
        case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
            std::cerr << "include nested too deeply: ";
            break;
        case simplecpp::Output::SYNTAX_ERROR:
            std::cerr << "syntax error: ";
            break;
        case simplecpp::Output::PORTABILITY_BACKSLASH:
            std::cerr << "portability: ";
            break;
        case simplecpp::Output::UNHANDLED_CHAR_ERROR:
            std::cerr << "unhandled char error: ";
            break;
        case simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND:
            std::cerr << "explicit include not found: ";
            break;
        }
        std::cerr << output.msg << std::endl;
    }

    // cleanup included tokenlists
    simplecpp::cleanup(included);
    

    return 0;
}
int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    chdir(args.front().data());
    bool pp = false;
    if (args.back() == "pp") {
        pp = true;
    }
    args.pop_back();
    std::string doing = args.back();
    args.pop_back();
    //fmt::print("doing:{}\n", doing);
    //simplecpp2(argc, argv);
    std::vector<Dep> deps;
    std::vector<fs::path> dirs;

    //concat all arguments with spaces
    std::string all_args;
    std::string all_args_clang;
    std::string out_resp;
    std::string in_cpp;
    fs::path in_filename;
    fs::path pp_filename;
    fs::path resp_filename;
    fs::path out_filename;
    bool resp_as_string = true;
    char delim = ' ';
    if (!resp_as_string) {
        delim = '\n';
    }
    int i = 0;
    std::string debug;
    auto transformArg = [](std::string_view arg) -> std::string {
        if (arg.ends_with("\r\n")) arg = arg.substr(0, arg.size() - 2);
        else if (arg.back() == '\n') arg = arg.substr(0, arg.size() - 1);
        else if (arg.back() == '\r') arg = arg.substr(0, arg.size() - 1);
        if (arg.starts_with("/I"))
            return std::string("-I") + std::string(arg.substr(sizeof("/I") - 1, arg.size()));
        else if (arg.starts_with("/FI"))
            return std::string("-include") + std::string(arg.substr(sizeof("/FI") - 1, arg.size()));
        else if (arg.starts_with("/D"))
            return std::string("-D") + std::string(arg.substr(sizeof("/D") - 1, arg.size()));
        else if (arg.starts_with("/imsvc"))
            return std::string("-isystem") + std::string(arg.substr(sizeof("/imsvc") - 1, arg.size()));
        return "";
    };
    auto getDir = [&deps, &dirs](std::string_view arg) -> void {
        if (arg.ends_with("\r\n")) arg = arg.substr(0, arg.size() - 2);
        else if (arg.back() == '\n') arg = arg.substr(0, arg.size() - 1);
        else if (arg.back() == '\r') arg = arg.substr(0, arg.size() - 1);
        if (arg.back() == '\"') arg = arg.substr(0, arg.size() - 1);
        if (arg.starts_with("/I")) {
            arg = arg.substr(sizeof("/I") - 1, arg.size());
            while (arg.front() == ' ') arg = arg.substr(1);
            if (arg.front() == '\"') arg = arg.substr(1);
            dirs.emplace_back(std::string(arg));
        }
        else if (arg.starts_with("/FI")) {
            arg = arg.substr(sizeof("/FI") - 1, arg.size());
            while (arg.front() == ' ') arg = arg.substr(1);
            if (arg.front() == '\"') arg = arg.substr(1);
            deps.emplace_back(Dep{ "", std::string(arg), "", "", 0, 0, 0});
        }
        else if (arg.starts_with("/imsvc")) {
            arg = arg.substr(sizeof("/imsvc") - 1, arg.size());
            while (arg.front() == ' ') arg = arg.substr(1);
            if (arg.front() == '\"') arg = arg.substr(1);
            dirs.emplace_back(std::string(arg));
        }
    };
    std::vector<std::string> _split;
    std::vector<const char*> _split2;
    std::unordered_set<std::string> set;
    auto start = timer_start();
    for (auto &arg : args) {
        debug += arg + " ";
        if (i++ == 0 && arg.front() != '@') continue;
        if (arg.front() == '@') {
            auto resp = readFile(arg.substr(1));
            if (resp.size() == 0) exit(10);
            _split = split(resp, '\n');
            int ii = 0;
            in_filename = fs::path(_split[0]);
            pp_filename = in_filename.replace_extension(".i");
            //out_resp += pp_filename.generic_string() + delim;
            for (auto& v : _split) {
                if (ii++ == 0) {
                    all_args_clang += v + " ";
                    in_cpp = v;
                    if (in_cpp.back() == '\r')
                    in_cpp.pop_back();
                    continue;
                }
                all_args_clang += transformArg(v) + " ";
                getDir(v);
                if (v.starts_with("/Fo")) {
                    if (v.back() == '\r') v.pop_back();
                    if (v.back() == '\"')
                        out_filename = v.substr(4, v.size() - 5);
                    else
                        out_filename = v.substr(3);
                }
                if (v.starts_with("/imsvc")/* || v.starts_with("/FI") */|| v.starts_with("/I") /*|| v.starts_with("/D")*/) {
                    replace(v, "/imsvc", "-isystem");
                    replace(v, "/FI", "-include");
                    replace(v, "/I", "-I");
                    replace(v, "\r", "");
                    replace(v, "\"", "");
                    replace(v, "-I ", "-I");
                    replace(v, "/D", "-D");
                    replace(v, "-D ", "-D");
                    replace(v, "-isystem ", "-isystem");
                    /*if (v.starts_with("-D")) {
                        auto _split3 = split(v, '=');
                        if (_split3.size() > 1) {
                            _split2.push_back(_split3[0].data());
                            _split2.push_back(_split3[1].data());
                        }
                        _split2.push_back(v.data());
                        continue;
                    } else*/
                    _split2.push_back(v.data());
                    continue;
                }
                if (v.starts_with("/imsvc") || v.starts_with("/clang:-MD") || v.starts_with("/FI") || v.starts_with("/Yu") || v.starts_with("-Werror") || v.starts_with("/Yc") || v.starts_with("/I") || v.starts_with("/Fo") || v.starts_with("/Fp") || v.starts_with("/sourceDependencies") || v.starts_with(".response") || v.starts_with("response")) {
                    continue;
                }
                if (v.back() == '\r') v.pop_back();
                while (v.front() == ' ') v = v.substr(1);
                set.emplace(v);
            }
            out_resp += "/wd4668 ";
            if (out_resp.size() > 255) resp_as_string = false;
            if (!resp_as_string) {
                resp_filename = in_filename.replace_extension(".i.response");
                writeFile(resp_filename.generic_string(), out_resp);
            }
        }
        all_args += arg + " ";
    }
    for (auto& v : set)
        out_resp += v + delim;

    all_args += "/wd4668 ";
    auto errorCallback = [](const TErrorInfo&)
    {
        fmt::print("Error pp\n");
    };
#undef max
    auto clangPP = [&_split2](const std::string& in_cpp) {
        CXIndex idx = clang_createIndex(1, 0);

        int line = -1;
        int column = -1;


        // Parse the source file into a translation unit
        CXTranslationUnit tu;
        _split2.push_back("-std=c++17");
        _split2.push_back("-Xclang");
        _split2.push_back("-includeD:\\ue5\\Engine\\Intermediate\\Build\\Win64\\UnrealHeaderTool\\Development\\CoreUObject\\PCH.CoreUObject.h");
        //_split2.push_back("D:\\ue5\\Engine\\Intermediate\\Build\\Win64\\UnrealHeaderTool\\Development\\CoreUObject\\PCH.CoreUObject.h");
        //_split2.push_back("D:\\ue5\\Engine\\Intermediate\\Build\\Win64\\UnrealHeaderTool\\Development\\CoreUObject\\PCH.CoreUObject.h");
        //_split2.push_back("--includeD:/ue5/Engine/Intermediate/Build/Win64/UnrealHeaderTool\/Development/CoreUObject/PCH.CoreUObject.h");
        _split2.push_back("-DUE_BUILD_DEVELOPMENT=1");
        _split2.push_back("-DWITH_EDITOR=1");
        _split2.push_back("--driver-mode=c++");
        _split2.push_back("..\\Intermediate\\Build\\Win64\\UnrealHeaderTool\\Development\\CoreUObject\\Module.CoreUObject.6_of_10.cpp");
        const auto clangArgsCount = _split2.size();

        CXErrorCode err = clang_parseTranslationUnit2(idx,
            in_cpp.data(),
            _split2.data(), clangArgsCount - 4,
            nullptr, 0,
            CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles | CXTranslationUnit_Incomplete, &tu);
        if (!tu)
        {
            printf("Couldn't parse tu\n");
            clang_disposeIndex(idx);
            return 1;
        }
        clang_getInclusions(tu, [](CXFile name, CXSourceLocation* loction, unsigned int, CXClientData ClientData) {
            CXString str = clang_getFileName(name);
            printf("include: %s\n", clang_getCString(str));
            }, NULL);

        // Show any diagnostic information if available
        for (unsigned int i = 0; i < clang_getNumDiagnostics(tu); i++)
        {
            CXDiagnostic diag = clang_getDiagnostic(tu, i);
            CXString diagSpelling = clang_getDiagnosticSpelling(diag);

            const auto location = clang_getDiagnosticLocation(diag);
            CXFile file;
            unsigned int offset = 0;
            unsigned int col = 0;
            unsigned int line = 0;
            clang_getSpellingLocation(location, &file, &line, &col, &offset);
            const CXString fileName = clang_getFileName(file);
            printf("DIAGNOSTIC: %s:%u:%u: %s\n", clang_getCString(fileName), line, col, clang_getCString(diagSpelling));
            clang_disposeString(fileName);
            clang_disposeString(diagSpelling);
            clang_disposeDiagnostic(diag);
        }
        printf("Total diagnostics available: %d\n", clang_getNumDiagnostics(tu));
        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(idx);
    };
    writeFile("test5.cpp", "int main() {return 1;}");
    /*auto deps2 = getIncludes(readFile(in_cpp), in_cpp);
    deps.insert(std::end(deps), std::begin(deps2), std::end(deps2));
    while (1) {
        auto deps2 = checkDeps(dirs, deps);
        if (deps2.size() > 0) {
            deps.insert(std::end(deps), std::begin(deps2), std::end(deps2));
        }
    }*/
    std::string vs2019 = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\MSVC\\14.29.30133\\bin\\Hostx64\\x64\\cl.exe";
    std::string vs2022 = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.32.31326\\bin\\Hostx64\\x64\\cl.exe";
    std::string clang_cl = "C:\\llvm-project\\build\\Release\\bin\\clang-cl.exe";
    std::string clang = "C:\\llvm-project\\build\\Release\\bin\\clang.exe";
    std::string llvm_rc = "C:\\llvm-project\\build\\Release\\bin\\llvm-rc.exe";
    std::string ispc = "D:\\ue5\\Engine\\Source\\ThirdParty\\Intel\\ISPC\\bin\\Windows\\ispc.exe";
    
    //std::string cmd1 = fmt::format("\"\"{}\" {}\"", clang_cl, all_args);
    //auto buf22 = exec2(cmd1.data());
    //return 1;
    //random temp name
    std::string cmd = fmt::format("\"\"{}\" {} /Fi\"{}\" /P {} /EP\"", clang_cl, all_args, pp_filename.generic_string(), doing == "cpp" ? "/TP" : "");
    auto randomnumber = rand() % 100;
    std::string cmd1 = fmt::format("\"\"{}\" {} -std=c++17 -M -MF{}.d \"", clang, all_args_clang, randomnumber);
    std::string cmd3 = "";
    if (doing == "cpp") {
        cmd3 = fmt::format("\"\"{}\" {} /Fi\"{}\" -Xclang -xcpp-output /P /EP \"", clang_cl, all_args, pp_filename.generic_string());
    }
    else if (doing == "c") {
        cmd3 = fmt::format("\"\"{}\" {} /Fi\"{}\" /P /EP \"", clang_cl, all_args, pp_filename.generic_string());
       // cmd3 = fmt::format("\"\"{}\" {} -E -o{}.i -P -w \"", clang, all_args_clang, randomnumber);
    }
    else if (doing == "ispc") {
        cmd3 = fmt::format("\"\"{}\" {} -E -o{}.i \"", ispc, all_args_clang, randomnumber);
    }
    else if (doing == "rc") {
        cmd3 = fmt::format("\"\"{}\" {} \"", llvm_rc, all_args_clang);
        auto buf = exec2(cmd3.data());
        return 300;
    }
    else {
        fmt::print("Unknown type {}\n", doing);
        return 501;
    }
    //delete temp

    /*std::string inputSource = "#include <system>\n#include \"non_system_path\"\n void main()\n{\n\treturn ADD(2, 3);\n}";
    StringInputStream input(inputSource);
    Lexer lexer(input);
    Preprocessor preprocessor(lexer, errorCallback);
    std::cout << preprocessor.Process() << std::endl;*/

    //fmt::print("cmd: {}\n", cmd);
    auto end = (float)timer_end(start);
    startProfile(end, out_filename.filename().generic_string());
    writeFile("debug.txt", debug);

    if (out_resp.size() == 0) return 11;

    //preprocess
    if (1) {
        auto start = timer_start();
        auto buf = exec2(cmd3.data());
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("preprocess", end);
    }

    if (0) {
        auto start = timer_start();
        for (int i = 0; i < 50; ++i) {
        auto buf = exec2(cmd3.data());
        }
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("preprocess", end);
    }

    //get deps using clang.exe
    if (0) {
        auto start = timer_start();
        for (int i = 0; i < 50; ++i) {
            auto buf = exec2(cmd1.data());
        }
        auto deps = readFile(fmt::format("{}.d", randomnumber));
        fs::remove(fmt::format("{}.d", randomnumber));
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("get deps (clang.exe)", end);
    }


    //get deps using libclang //too slow
    if (0) {
        auto start = timer_start();
        //auto end2 = (float)timer_end(start2);
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("get deps (libclang)", end);
    }

    //get deps using own parser
    if (0) {
        auto start = timer_start();

        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("get deps (libclang)", end);
    }

#if DOING_LOCAL || DOING_VERIFY
    executor.silent_async([&] {
        std::string temp_cmd = fmt::format("\"\"{}\" \"{}\" \"{}\" /Fo\"{}\" /TP /c\"", clang_cl, pp_filename.generic_string(), resp_as_string ? out_resp : "@" + resp_filename.generic_string(), out_filename.generic_string());
        writeFile("debug.txt", temp_cmd);
        auto buf2 = exec2(temp_cmd.data());
        //writeFile(out_filename.generic_string(), buf2);
    });
#endif
#if !DOING_LOCAL
    std::string temp;

    {
        auto start = timer_start();
        temp = readFile(pp_filename.generic_string());
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("readI", end);
    }
    
    if (temp.size() == 0) return 2;

    //compress temp
    {
        auto start = timer_start();
        temp = Compressor::ZSTD->Compress(temp, 11);
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("compressI", end);
    }
    if (temp.size() == 0) return 3;

    //call a process
    const std::string remote = "http://ec2-44-234-71-191.us-west-2.compute.amazonaws.com/";
    const std::string remote2 = "http://ec2-35-172-192-246.compute-1.amazonaws.com:30000/";
    const std::string coord = "http://remotecoord.theimmersiveweb.workers.dev";
    //out_resp = Compressor::ZSTD->Compress(out_resp, 17);
    //compress temp
    std::string enc;
    {
        auto start = timer_start();
        enc = macaron::Base64::Encode(out_resp);
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("encodeCMD", end);
    }

    std::vector<u64> counts(2);
    cpr::Response r2;
    std::string server = "";
    std::string uuid = "";
    if (1) {
        int i = 0;
        for (auto _server : { remote2 }) {
            auto start = timer_start();
            r2 = Get(Url{ _server },
                Header{ {"type", "ping"} },
                Body{ temp }
            );
            auto end = (float)timer_end(start);
            if (r2.text != "") {
                server = _server;
                uuid = r2.text;
                break;
            }
        }
    }
    if (server == "" || uuid == "") {
        fmt::print("Error 100, failed to find an available server\n");
        return 100;
    }
    cpr::Response r3;
    {
        auto start = timer_start();
        int i = 0;
        r3 = Post(Url{ server },
            Header{ {"Content-Type", "application/octet-stream"}, {"type", "msvc"}, {"cmd", enc}, {"uuid", uuid}, {"pp", pp ? "1" : "0"} , {"doing", doing} },
            Body{ temp }
        );
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("send/recv", end);
    }

    auto getU64 = [&r3](const std::string& str) {
        if (!r3.header.contains(str)) return 0ull;
        uint64_t value;
        std::istringstream iss(r3.header.at(str));
        iss >> value;
        return value;
    };

    if (pp) {
        uint64_t value = getU64("job");
        return value;
    }
    
    if (r3.status_code == 503) {
        fmt::print("Error 503: {}\n", r3.text);
        return 503;
    }
    if (!r3.header.contains("size")) {
        fmt::print("Error missing header, returned length: {} returned data: {} returned code: {}\n", r3.text.size(), (r3.text.size() < 1 * 1024 * 1024 && r3.text.size() > 0) ? "returned data: " + r3.text : "", r3.status_code);
        return 4;
    }
    if (r3.text.size() == 0) return 5;

    profile("decompressI", getU64("decompressI"));
    profile("writeI", getU64("writeI"));
    profile("compileOBJ", getU64("compileOBJ"));
    profile("compressOBJ", getU64("compressOBJ"));
    profile("readOBJ", getU64("readOBJ"));

    //decompress obj
    uint64_t value = getU64("size");
    {
        auto start = timer_start();
        Compressor::ZSTD->Decompress(r3.text, value);
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("decompressOBJ", end);
    }
#if DOING_VERIFY
    executor.wait_for_all();
    auto r1 = readFile(out_filename.generic_string());
    if (r3.text.size() != r1.size()) {
        fmt::print("Remote does not equal to local object\n");
        //return 7;
    }
#endif
    if (r3.text.size() == 0) return 6;
    {
        auto start = timer_start();
        writeFile(out_filename.generic_string(), r3.text);
        auto end = (float)timer_end(start);
        appendProfile(end);
        profile("writeOBJ", end);
    }
#endif



    //track
    //time to read from disk
    //time to precompile
    //time to compress
    //time to send/recv (includes time to decompress, compile and 2nd compress)
    //time to decompress
    //time to write to disk
    //bandwidth with and without compression

    //ubt track start and actions, do normal amount of actions locally and the rest remotely
#if PROFILING == 1
    fmt::print("Profile: {}\n", profileOut);
#endif
    return 0;
}