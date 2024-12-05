#include "threadpool.hpp"
#include <stdexcept>
#include <gtest/gtest.h>

int values[100];
int nested_sum = 0;
int multi_arg_sum = 0;
std::mutex sum_mutex;

void testFunc(ThreadPool<int>* pool, int& index) {
    values[index] = index;
}

// Test queueing the pool during jobs
void testNestedFunc(ThreadPool<int>* pool, int& a) {
    if (a > 1) {
        int new_int = a - 1;
        pool->QueueJob(new_int);
        pool->QueueJob(new_int);
    }
    // Critical section
    std::lock_guard lk(sum_mutex);
    nested_sum += a;
    // End critical section
}

void multiArgFunc(ThreadPool<int, std::string>* pool, int& a, std::string& b) {
    if (a > 1) {
        int new_int = a - 1;
        std::string new_str(b + "a");
        pool->QueueJob(new_int, new_str);
        pool->QueueJob(new_int, new_str);
    }

    // Critical section
    std::lock_guard lk(sum_mutex);
    multi_arg_sum += b.length();
    // End critical section
}

void threadFuncRuntimeException(ThreadPool<int>* pool, int& a) {
    throw std::runtime_error("Runtime error");
}

void threadFuncInvalidArgException(ThreadPool<int>* pool, int& a) {
    throw std::invalid_argument("Invalid arg exception");
}

int sumValues() {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += values[i];
    }
    return sum;
}

TEST(ThreadPoolTests, TestThreadPool) {
    ThreadPool<int> pool(3, testFunc);
    for (int i = 0; i < 100; i++) {
        pool.QueueJob(i);
    }
    ASSERT_EQ(sumValues(), 0);
    pool.Start();
    pool.Join();
    ASSERT_EQ(sumValues(), 4950); // sum of 0 to 99
}

TEST(ThreadPoolTests, TestThreadPoolNested) {
    ThreadPool<int> pool(3, testNestedFunc);
    int job = 10;
    pool.QueueJob(job);
    ASSERT_EQ(nested_sum, 0);
    pool.Start();
    pool.Join();
    int want = 10 + (9*2) + (8*4) + (7*8) + (6*16) + (5*32) + (4*64) + (3*128) + (2*256) + (1*512);
    ASSERT_EQ(nested_sum, want);
}

TEST(ThreadPoolTests, TestThreadPoolMultipleArgs) {
    ThreadPool<int, std::string> pool(3, multiArgFunc);
    int job_int = 10;
    std::string job_str = "test";
    pool.QueueJob(job_int, job_str);
    ASSERT_EQ(multi_arg_sum, 0);
    pool.Start();
    pool.Join();
    int want = 4 + (5*2) + (6*4) + (7*8) + (8*16) + (9*32) + (10*64) + (11*128) + (12*256) + (13*512);
    ASSERT_EQ(multi_arg_sum, want);
}

TEST(ThreadPoolTests, TestThreadPoolJobException) {
    ThreadPool<int> pool(3, threadFuncRuntimeException);
    ThreadPool<int> pool2(3, threadFuncInvalidArgException);
    for (int i = 0; i < 100; i++) {
        pool.QueueJob(i);
        pool2.QueueJob(i);
    }
    pool.Start();
    pool2.Start();
    pool.Join();
    pool2.Join();
}