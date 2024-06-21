#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

constexpr int VECTOR_SIZE = 25000000;
constexpr int NUM_THREADS = 4;

void generate_random_data(std::vector<int>& numbers) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, VECTOR_SIZE);
    for (auto& elem : numbers) {
        elem = dist(gen);
    }
}

void printExecutionTime(const std::chrono::steady_clock::time_point& startTime,
                        const std::chrono::steady_clock::time_point& endTime,
                        const std::string& msg) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        endTime - startTime)
                        .count();
    std::cout << msg << ": " << duration << " миллисекунд\n";
}

void sortThread(std::vector<int>& vec, int start, int end) {
    std::sort(vec.begin() + start, vec.begin() + end);
}

void parallelSort(std::vector<int>& vec) {
    int chunkSize = vec.size() / NUM_THREADS;
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        int start = i * chunkSize;
        int end = (i == NUM_THREADS - 1) ? vec.size() : (i + 1) * chunkSize;
        threads.emplace_back(sortThread, std::ref(vec), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int step = chunkSize; step < vec.size(); step *= 2) {
        for (int i = 0; i < vec.size(); i += 2 * step) {
            int start = (i + step < vec.size()) ? i + step : vec.size();
            int end = (i + 2 * step < vec.size()) ? i + 2 * step : vec.size();
            std::inplace_merge(vec.begin() + i, vec.begin() + start,
                               vec.begin() + end);
        }
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(1, VECTOR_SIZE);

    std::vector<int> numbers(VECTOR_SIZE);
    generate_random_data(numbers);

    auto dataCopy = numbers;
    auto start = std::chrono::steady_clock::now();
    std::sort(dataCopy.begin(), dataCopy.end());
    auto end = std::chrono::steady_clock::now();
    printExecutionTime(start, end, "Single-threaded sort time");

    auto dataCopySecond = numbers;
    start = std::chrono::steady_clock::now();
    parallelSort(dataCopySecond);
    if (dataCopySecond != dataCopy) {
        std::cerr << "Ошибка: массивы неодинаково отсортированы\n";
        return 1;
    }
    end = std::chrono::steady_clock::now();
    printExecutionTime(start, end, "Multi-threaded manual sort time");

    auto dataCopyThird = numbers;
    start = std::chrono::steady_clock::now();
    std::sort(std::execution::par, dataCopyThird.begin(), dataCopyThird.end());
    if (dataCopySecond != dataCopy || dataCopyThird != dataCopySecond ||
        dataCopyThird != dataCopy) {
        std::cerr << "Ошибка: массивы неодинаково отсортированы\n";
        return 1;
    }
    end = std::chrono::steady_clock::now();
    printExecutionTime(start, end,
                       "Multi-threaded std::execution::par sort time");

    return 0;
}