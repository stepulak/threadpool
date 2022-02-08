#define BOOST_TEST_MODULE threadpool_test
#include <boost/test/unit_test.hpp>

#include "threadpool.hpp"

#include <cmath>

using namespace std::chrono_literals;

BOOST_AUTO_TEST_CASE(threadpool_number_of_threads)
{
    {
        thread_pool pool { 1 };
        BOOST_REQUIRE_EQUAL(pool.number_of_threads(), 1u);
    }
    {
        thread_pool pool;
        BOOST_REQUIRE_EQUAL(pool.number_of_threads(), std::thread::hardware_concurrency());
    }
}

BOOST_AUTO_TEST_CASE(threadpool_queue_work)
{
    thread_pool pool;
    std::atomic_bool work_done { false };
 
    pool.queue_work([&] {
        work_done = true;
    });

    // Not bulletproof, but simple wait for thread to finish
    std::this_thread::sleep_for(50ms);

    BOOST_REQUIRE_EQUAL(work_done, true);
    BOOST_REQUIRE_EQUAL(pool.work_queue_size(), 0);
}

BOOST_AUTO_TEST_CASE(threadpool_queue_work_blocking)
{
    thread_pool pool { 1 };
    std::atomic_int work_result { 0 };

    pool.queue_work([&work_result] {
        work_result += 1;
        std::this_thread::sleep_for(100ms);
    });
    pool.queue_work([&work_result] {
        work_result += 2;
    });
    pool.queue_work([&work_result] {
        work_result += 4;
    });
    pool.queue_work([&work_result] {
        work_result += 8;
    });

    std::this_thread::sleep_for(50ms);

    BOOST_REQUIRE_EQUAL(work_result, 1);
    BOOST_REQUIRE_EQUAL(pool.work_queue_size(), 3);

    std::this_thread::sleep_for(150ms);

    BOOST_REQUIRE_EQUAL(work_result, 15);
}

BOOST_AUTO_TEST_CASE(threadpool_queue_work_semiblocking)
{
    thread_pool pool { 2 };
    std::atomic_int work_result { 0 };

    for (int i = 0; i < 5; i++) {
        pool.queue_work([i, &work_result] {
            work_result += static_cast<int>(std::pow(2, i));
            std::this_thread::sleep_for(30ms);
        });
    }

    std::this_thread::sleep_for(50ms);

    BOOST_REQUIRE_LT(work_result, 31);
}

BOOST_AUTO_TEST_CASE(threadpool_queue_work_nonblocking)
{
    thread_pool pool { 4 };
    std::atomic_int work_result { 0 };

    for (int i = 0; i < 4; i++) {
        pool.queue_work([i, &work_result] {
            work_result += static_cast<int>(std::pow(2, i));
            std::this_thread::sleep_for(30ms);
        });
    }

    std::this_thread::sleep_for(50ms);

    BOOST_REQUIRE_EQUAL(work_result, 15);
    BOOST_REQUIRE_EQUAL(pool.work_queue_size(), 0);
}