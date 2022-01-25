#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

class thread_pool {
public:
    using work_function_t = std::function<void()>;

    thread_pool()
        : thread_pool(std::thread::hardware_concurrency())
    {
    }

    thread_pool(size_t number_threads)
        : _running { true }
    {
        const auto thread_fn = [&] {
            while (_running) {
                std::unique_lock<std::mutex> lock { _mutex };
                _cv.wait(lock);
                const auto work = _work_queue.front();
                _work_queue.pop();
                lock.unlock();
                work();
            }
        };

        _threads.resize(number_threads);
        for (size_t i = 0u; i < number_threads; i++) {
            _threads[i] = std::make_unique<std::thread>(thread_fn);
        }
    }

    ~thread_pool() { stop(); }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    thread_pool& queue_work(work_function_t work)
    {
        std::unique_lock<std::mutex> lock { _mutex };
        _work_queue.emplace(std::move(work));
        lock.unlock();
        _cv.notify_one();
    }

    void stop() {
        _running = false;
        for (const auto& p : _threads) {
            p->join();
        }
    }

    size_t number_of_threads() const { return _threads.size(); }

private:
    std::atomic<bool> _running;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<work_function_t> _work_queue;
    std::vector<std::unique_ptr<std::thread>> _threads;
};