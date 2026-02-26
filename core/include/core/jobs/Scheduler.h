#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace core::jobs {

class SlotLimiter {
public:
    explicit SlotLimiter(std::size_t slots);

    void acquire();
    void release();

private:
    std::size_t slots_;
    std::size_t available_;
    std::mutex m_;
    std::condition_variable cv_;
};

class Scheduler {
public:
    using Task = std::function<void()>;

    Scheduler(std::size_t workers, std::size_t queueCapacity);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void enqueue(Task t);
    void stop();

private:
    void workerLoop();

    std::deque<Task> q_;
    std::size_t cap_;
    std::mutex m_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;

    bool stopping_ = false;
    std::vector<std::thread> workers_;
};

}
