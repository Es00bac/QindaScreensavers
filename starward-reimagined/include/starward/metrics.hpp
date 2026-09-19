// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
namespace starward {
struct CpuCount {std::uint64_t total{},idle{};};
struct NetCount {std::uint64_t rx{},tx{};};
std::optional<CpuCount> parseCpu(const std::string&);
std::optional<double> cpuUsage(CpuCount old,CpuCount now);
std::optional<std::pair<double,double>> parseMemory(const std::string&);
std::map<std::string,NetCount> parseNetwork(const std::string&);
std::string defaultInterface(const std::string& route);
struct Metrics {std::optional<double> cpu,ramUsed,ramTotal,rx,tx,uptime; std::string interface; bool demo=false,privateMode=false; std::chrono::steady_clock::time_point sampled{};};
Metrics demoMetrics(double t);
class Monitor {
public:
 explicit Monitor(std::string interface={}); ~Monitor();
 Monitor(const Monitor&)=delete; Monitor&operator=(const Monitor&)=delete;
 Metrics snapshot() const;
private:
 void run(); std::string selected_; mutable std::mutex mutex_; std::condition_variable wake_;
 bool stop_=false; Metrics current_; std::thread worker_;
};
}
