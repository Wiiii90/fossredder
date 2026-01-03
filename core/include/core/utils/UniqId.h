#pragma once

#include <atomic>
#include <chrono>
#include <string>

namespace utils {

inline std::string makeUniqId()
{
    static std::atomic<uint32_t> counter{0};
    const auto now = std::chrono::system_clock::now();
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    const uint32_t c = counter.fetch_add(1, std::memory_order_relaxed);
    return std::to_string(us) + "_" + std::to_string(c);
}

} // namespace utils
