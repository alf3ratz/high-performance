#include <cmath>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

const double MIN_REAL = -2.0;
const double MAX_REAL = 1.0;
const double MIN_IMAGINARY = -1.5;
const double MAX_IMAGINARY = 1.5;

const int MAX_ITERATIONS = 1000;

std::mutex taskQueueMutex;
std::condition_variable taskQueueCV;
std::queue<int> taskQueue;

// Массив для хранения цветов точек изображения
std::vector<int> colors(WIDTH* HEIGHT);

int setColor(int iterations) {
    if (iterations == MAX_ITERATIONS) {
        return 0;
    }
    return (int)((iterations % 256) * 255.0 / 255);
}

// Функция для вычисления фрактала Мандельброта в строке изображения
void computeMandelbrotRow(int row) {
    double realStep = (MAX_REAL - MIN_REAL) / WIDTH;
    double imaginaryStep = (MAX_IMAGINARY - MIN_IMAGINARY) / HEIGHT;

    double cImaginary = MAX_IMAGINARY - row * imaginaryStep;

    for (int x = 0; x < WIDTH; ++x) {
        double cReal = MIN_REAL + x * realStep;

        double zr = 0.0;
        double zi = 0.0;

        int iterations = 0;
        while (zr * zr + zi * zi <= 4.0 && iterations < MAX_ITERATIONS) {
            double temp = zr * zr - zi * zi + cReal;
            zi = 2.0 * zr * zi + cImaginary;
            zr = temp;
            iterations++;
        }

        colors[row * WIDTH + x] = setColor(iterations);
    }
}

void workThread() {
    while (true) {
        int task = -1;
        // Получение задания из очереди
        {
            std::lock_guard<std::mutex> lock(taskQueueMutex);

            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
            }
        }
        if (task == -1) break;

        // Выполяем строку изображения
        std::lock_guard<std::mutex> lock(taskQueueMutex);
        computeMandelbrotRow(task);
    }
}
void saveImagePPM(const std::string& filename, const std::vector<int>& pixels,
                  int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка при открытии файла\n";
        return;
    }

    file << "P6\n";
    file << width << " " << height << "\n";
    file << "255\n";

    for (int i = 0; i < width * height; ++i) {
        unsigned char r = pixels[i] % 256;
        unsigned char g = pixels[i] % 256;
        unsigned char b = pixels[i] % 256;
        file << r << g << b;
    }

    file.close();
}

int main() {
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(workThread);
    }

    // Заполненяем очередь заданиями - номерами строк
    for (int y = 0; y < HEIGHT; ++y) {
        taskQueue.push(y);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    saveImagePPM("mandelbrot_threadpool.ppm", colors, WIDTH, HEIGHT);
    return 0;
}
