#include <algorithm>
#include <execution>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

constexpr int VECTOR_SIZE = 25000000; 
constexpr int NUM_THREADS = 8;

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
    std::cout << msg << ": " << duration << " милисек\n";
}

void sortThread(std::vector<int>& numbers, int startIndex, int endIndex) {
    std::vector<int> sortedChunk(numbers.begin() + startIndex,
                                 numbers.begin() + endIndex);
    std::sort(sortedChunk.begin(), sortedChunk.end());
    numbers = std::move(sortedChunk);
}
void parallel_sort(std::vector<int>& numbers) {
    std::vector<std::vector<int>> segmentedVectors(NUM_THREADS);
    const int segmentSize = VECTOR_SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        segmentedVectors[i] =
            std::vector<int>(numbers.begin() + i * segmentSize,
                             numbers.begin() + (i + 1) * segmentSize);
    }

    // Запуск сортировки на каждом потоке
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        int startIndex = i * segmentSize;
        int endIndex =
            (i == NUM_THREADS - 1) ? VECTOR_SIZE : (i + 1) * segmentSize;
        threads.emplace_back(sortThread, std::ref(segmentedVectors[i]),
                             startIndex, endIndex);
    }

    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "aboba\n";
    std::vector<int> sortedNumbers;
    sortedNumbers.reserve(numbers.size());
    for (const auto& chunk : segmentedVectors) {
        sortedNumbers.insert(sortedNumbers.end(), chunk.begin(), chunk.end());
    }
    std::inplace_merge(sortedNumbers.begin(),
                       sortedNumbers.begin() + segmentSize,
                       sortedNumbers.end());

    numbers = std::move(sortedNumbers);
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
    parallel_sort(dataCopySecond);
    end = std::chrono::steady_clock::now();
    printExecutionTime(start, end, "Multi-threaded manual sort time");

    auto dataCopyThird = numbers;
    start = std::chrono::steady_clock::now();
    std::sort(std::execution::par, dataCopyThird.begin(), dataCopyThird.end());
    end = std::chrono::steady_clock::now();
    printExecutionTime(start, end,
                       "Multi-threaded std::execution::par sort time");

    return 0;
}