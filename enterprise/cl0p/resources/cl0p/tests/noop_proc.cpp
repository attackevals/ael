#include <chrono>
#include <thread>

#define SLEEP_TIME_MS 60000

int main() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    }
    return 0;
}