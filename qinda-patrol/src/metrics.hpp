#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace patrol {
enum class MetricMode { Live, Demo, Hidden };
struct Metrics {
    MetricMode mode=MetricMode::Live;
    double cpu=-1, memory=-1, memoryUsedGiB=-1, memoryTotalGiB=-1;
    double load1=-1, uptime=-1, rxKiB=-1, txKiB=-1;
    std::string interface;
    std::chrono::steady_clock::time_point sampled{};
};
struct CpuCounters { std::array<std::uint64_t,8> values{}; };
std::optional<CpuCounters> parseCpu(const std::string& text);
std::optional<double> cpuPercent(const CpuCounters& before,const CpuCounters& after);
void parseMemory(const std::string& text,Metrics& out);
std::string defaultInterface(const std::string& route);
std::optional<std::array<std::uint64_t,2>> parseNetwork(const std::string& text,const std::string& iface);
Metrics demoMetrics(double seconds);
// One read-only collector per desktop application. Never shells out, reads
// process names, resolves hosts, writes /proc, or queries external services.
class MetricSampler {
public:
    explicit MetricSampler(std::string interface={});
    ~MetricSampler();
    MetricSampler(const MetricSampler&)=delete;
    MetricSampler& operator=(const MetricSampler&)=delete;
    Metrics snapshot() const;
    void setActive(bool active);
private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::thread worker_;
    bool stop_=false,active_=true;
    Metrics latest_;
    std::string requestedInterface_;
    void run();
};
}
