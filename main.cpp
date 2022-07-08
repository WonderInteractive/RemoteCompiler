#include "json.hpp"
#include <cpr/cpr.h>
#include <fmt/format.h>
#include "helper.h"
#include "taskflow/taskflow.hpp"
#include "compressor.h"
using namespace cpr;
using json = nlohmann::json;
tf::Executor executor;
tf::Taskflow taskflow;
SetT<u64> doing;
void optimizeBinary(const u64 binary) {
    //download binary
    Url url{ "http://fetchbin.theimmersiveweb.workers.dev/" + std::to_string(binary) + ".wasm" };
    Session session;
    session.SetUrl(url);
    curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_HTTP_CONTENT_DECODING, 0);
    auto r = session.Get();
    if (r.status_code != 200) {
        fmt::print("Invalid binary {}, skipping\n", binary);
        return;
    }
    fmt::print("binary size in = {}\n", r.text.size());
    //brotli decompress binary
    Compressor::Brotli->Decompress(r.text, r.text.size());
    //compress binary
    auto compressed = Compressor::Brotli->Compress(r.text, 8);
    fmt::print("binary size out = {}\n", compressed.size());
    //upload binary
    Response r2 = cpr::Post(Url{ "http://uploadbin.theimmersiveweb.workers.dev/" },
        Header{ {"Content-Type", "application/octet-stream"}, {"type", "application/wasm"}, {"hash", std::to_string(binary) + ".wasm"}, {"optimized", "1"}},
        Body{ compressed }
    );
    doing.erase(binary);
}
std::vector<std::vector<datahash>> getAssetsWithinNSeconds(const std::vector<std::pair<datahash, u32>>& asset_ts, const u32 N) {
    auto starting_asset = asset_ts.front();
    auto starting_time = starting_asset.second;
    std::vector<std::vector<datahash>> buckets(1);
    u32 bucket_index = 0;
    for (int i = 1; i < asset_ts.size(); ++i) {
        if (asset_ts[i].second == 0) continue;
        if (asset_ts[i].second - starting_time < N) {
            buckets[bucket_index].push_back(asset_ts[i].first);
        }
        else {
            buckets.push_back({ asset_ts[i].first });
            bucket_index++;
            starting_time = asset_ts[i].second;
        }
    }
    return buckets;
}
struct Stats {
    u64 average;
    u64 median;
    u64 low;
    u64 high;
};
Stats getAverageMedianLowHigh(std::vector<u64>& values) {
    Stats stats;
    //get average of values
    u64 sum = 0;
    for (auto value : values) {
        sum += value;
    }
    stats.average = sum / values.size();
    //get median of values
    std::sort(values.begin(), values.end());
    if (values.size() % 2 == 0) {
        stats.median = (values[values.size() / 2] + values[values.size() / 2 - 1]) / 2;
    }
    else {
        stats.median = values[values.size() / 2];
    }
    //get low and high of values
    stats.low = values.front();
    stats.high = values.back();
    return stats;
}
void getSessions(const std::string& project, const std::string& rev) {
    auto r = cpr::Get(cpr::Url{ "http://getsessions.theimmersiveweb.workers.dev" },
        cpr::Header{ {"projectid", project}, {"rev", rev} }
    );
    if (r.status_code == 200) {
        json _json = json::parse(r.text);
        std::vector<u64> cpus;
        std::vector<u64> gpus;
        std::vector<u64> asset_sizes;
        std::vector<u64> asset_counts;
        std::vector<u64> startup_time_0s;
        std::vector<u64> startup_time_1s;
        std::vector<u64> total_times;
        u64 play_count = 0;
        u64 total_play_time = 0;
        for (auto v : _json) {
            play_count++;
            auto id = std::string(v["id"]);
            auto _cpu = v["cpu"];
            auto mem = v["mem"];
            auto gpu = v["gpu"];
#define strToU64(name) uint64_t name; { std::string name_ = v[#name]; std::istringstream iss(name_); iss >> name; }
            strToU64(asset_size);
            strToU64(startup_time_0);
            strToU64(startup_time_1);
            strToU64(total_time);
            total_play_time += total_time;

            auto asset_count = v["asset_count"];
            auto platform_session = v["platform_session"];
            auto r2 = cpr::Get(cpr::Url{ "http://session.theimmersiveweb.workers.dev" },
                cpr::Header{ {"get", id} }
            );
            std::vector<std::pair<datahash, u32>> asset_ts;
            Archive a(r2.text);
            a >> asset_ts;
            auto buckets = getAssetsWithinNSeconds(asset_ts, 5000);
            fmt::print("Status: {} {}\n", id, asset_ts.size());
            //add to stats
            cpus.push_back(_cpu);
            gpus.push_back(gpu);
            //get total play time
            try {
                asset_sizes.push_back(asset_size);
                asset_counts.push_back(asset_count);
                startup_time_0s.push_back(startup_time_0);
                startup_time_1s.push_back(startup_time_1);
                total_times.push_back(total_time);
            }
            catch (std::string err) {
                fmt::print("Error: {}\n", err);
            }
        }
        auto stats_cpu = getAverageMedianLowHigh(cpus);
        auto stats_gpu = getAverageMedianLowHigh(gpus);
        auto stats_asset_size = getAverageMedianLowHigh(asset_sizes);
        auto stats_asset_count = getAverageMedianLowHigh(asset_counts);
        auto stats_startup_time_0 = getAverageMedianLowHigh(startup_time_0s);
        auto stats_startup_time_1 = getAverageMedianLowHigh(startup_time_1s);
        auto stats_total_time = getAverageMedianLowHigh(total_times);
        fmt::print("Play Count: {}\n", play_count);
        fmt::print("Total Play Time: {}\n", total_play_time);
        fmt::print("CPU: {} {} {} {}\n", stats_cpu.average, stats_cpu.median, stats_cpu.low, stats_cpu.high);
        fmt::print("GPU: {} {} {} {}\n", stats_gpu.average, stats_gpu.median, stats_gpu.low, stats_gpu.high);
        fmt::print("Asset Size: {} {} {} {}\n", stats_asset_size.average, stats_asset_size.median, stats_asset_size.low, stats_asset_size.high);
        fmt::print("Asset Count: {} {} {} {}\n", stats_asset_count.average, stats_asset_count.median, stats_asset_count.low, stats_asset_count.high);
        fmt::print("Startup Time 0: {} {} {} {}\n", stats_startup_time_0.average, stats_startup_time_0.median, stats_startup_time_0.low, stats_startup_time_0.high);
        fmt::print("Startup Time 1: {} {} {} {}\n", stats_startup_time_1.average, stats_startup_time_1.median, stats_startup_time_1.low, stats_startup_time_1.high);
        fmt::print("Total Time: {} {} {} {}\n", stats_total_time.average, stats_total_time.median, stats_total_time.low, stats_total_time.high);
#define STR(x) #x
#define toJson(name) {STR(name)"_median", stats_##name.median},{STR(name)"_low", stats_##name.low},{STR(name)"_high", stats_##name.high},{STR(name)"_avg", stats_##name.average},
        json out_json = {
            toJson(cpu)
            toJson(gpu)
            toJson(asset_size)
            toJson(asset_count)
            toJson(startup_time_0)
            toJson(startup_time_1)
            toJson(total_time)
            { "total_play_time", total_play_time },
            { "play_count", play_count },
        };
        auto r3 = cpr::Post(cpr::Url{ "http://session.theimmersiveweb.workers.dev" },
            cpr::Header {
                {"action", "set"}, {"projectid", project }, {"rev", rev },
            },
            cpr::Body { out_json.dump()  }
        );
    }
}
int main() {
    //start an infinite loop that routinely makes a request to the server getting the latest sessions and binaries
    while (true) {
        auto r = cpr::Get(cpr::Url{ "http://getbinariesneedingopt.theimmersiveweb.workers.dev" },
            cpr::Header{ {"action", "get"} }
        );
        if (r.status_code == 200) {
            std::vector<u64> binaries = { (u64*)r.text.data(), (u64*)r.text.data() + r.text.size() / sizeof(u64) };
            for (auto v : binaries) {
                if (doing.contains(v)) continue;
                doing.emplace(v);
                fmt::print("Binary: {}\n", v);
                executor.silent_async([v]() { optimizeBinary(v); });
            }
        }
        auto r2 = cpr::Get(cpr::Url{ "http://getsessions.theimmersiveweb.workers.dev" },
            cpr::Header{ {"action", "list"} }
        );        
        if (r2.status_code == 200) {
            json _json = json::parse(r2.text);
            for (auto v : _json) {
                auto name = v["name"];
                std::string project = std::string(name);
                std::string rev = project.substr(36);
                project = project.substr(0, 36);
                fmt::print("Status: {} {}\n", project, rev);
                getSessions(project, rev);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }



    return 1;
}