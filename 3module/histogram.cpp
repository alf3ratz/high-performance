#include <array>
#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

const int HISTOGRAM_SIZE = 256;
const int NUM_THREADS = 4;

// Глобальная гистограмма для всех байтов (0-255)
std::array<std::atomic<int>, HISTOGRAM_SIZE> histogram;

std::mutex file_mutex;

void countBytes(const std::string& filename, size_t start, size_t end) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка при открытии файла\n";
        return;
    }

    file.seekg(start);
    std::vector<char> buffer(end - start);
    file.read(buffer.data(), buffer.size());

    for (char byte : buffer) {
        histogram[static_cast<unsigned char>(byte)]++;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Use file as argument\n";
        return 1;
    }

    std::string filename = argv[1];

    for (auto& count : histogram) {
        count = 0;
    }

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Ошибка при открытии файла\n";
        return 1;
    }

    int fileSize = file.tellg();
    file.close();

    std::vector<std::thread> threads;
    size_t chunkSize = fileSize / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == NUM_THREADS - 1) ? fileSize : start + chunkSize;
        threads.emplace_back(countBytes, std::ref(filename), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Вывод гистограммы
    for (int i = 0; i < HISTOGRAM_SIZE; ++i) {
        std::cout << i << ": " << histogram[i] << "\n";
    }

    return 0;
}