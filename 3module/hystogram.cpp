#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

const int HYSTOGRAM_SIZE = 256;
const int NUM_THREADS = 4;  // Количество потоков
std::mutex mutex;
void countHistogram(const std::string& filename, std::vector<int>& hystogram) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Opening file error\n";
        return;
    }

    char buffer[4096];
    int localHystogram[HYSTOGRAM_SIZE] = {};
    int bytesRead = 0;

    while (file) {
        file.read(buffer, sizeof(buffer));
        int count = file.gcount();
        bytesRead += count;

        for (int i = 0; i < count; i++) {
            ++localHystogram[static_cast<unsigned char>(buffer[i])];
        }
    }

    file.close();

    std::lock_guard<std::mutex> lock(mutex);
    for (int i = 0; i < HYSTOGRAM_SIZE; i++) {
        hystogram[i] += localHystogram[i];
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Use file as argument\n";
        return 1;
    }

    std::string filename = argv[1];

    std::vector<std::thread> threads;
    std::vector<int> hystogram(HYSTOGRAM_SIZE);

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(countHistogram, filename, std::ref(hystogram));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Гистограмма значений байтов:" << std::endl;
    for (int i = 0; i < HYSTOGRAM_SIZE; i++) {
        std::cout << i << ": " << hystogram[i] << std::endl;
    }

    return 0;
}
