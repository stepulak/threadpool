#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool {
public:
    using work_function_t = std::function<void()>;

    thread_pool(size_t number_threads = std::thread::hardware_concurrency())
    {
        const auto thread_fn = [&] {
            while (_running) {
                std::unique_lock<std::mutex> lock { _mutex };
                if (_work_queue.empty()) {
                    lock.unlock();
                    continue;
                }
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

    ~thread_pool()
    {
        _running = false;
        for (const auto& p : _threads) {
            p->join();
        }
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    thread_pool& queue_work(work_function_t work)
    {
        std::scoped_lock<std::mutex> lock { _mutex };
        _work_queue.emplace(std::move(work));
        return *this;
    }

    size_t number_of_threads() const
    {
        return _threads.size();
    }

    size_t work_queue_size() const
    {
        std::scoped_lock<std::mutex> lock { _mutex };
        return _work_queue.size();
    }

private:
    mutable std::mutex _mutex;
    std::atomic_bool _running = true;
    std::queue<work_function_t> _work_queue;
    std::vector<std::unique_ptr<std::thread>> _threads;
};