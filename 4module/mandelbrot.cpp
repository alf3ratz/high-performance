#include <omp.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

const double MIN_REAL = -2.0;
const double MAX_REAL = 1.0;
const double MIN_IMAGINARY = -1.5;
const double MAX_IMAGINARY = 1.5;

const int MAX_ITERATIONS = 1000;

std::vector<int> colors(WIDTH* HEIGHT);

int setColor(int iterations) {
    if (iterations == MAX_ITERATIONS) {
        return 0;
    }
    return (int)((iterations % 256) * 255.0 / 255);
}

void saveImagePPM(const std::string& filename, const std::vector<int>& pixels,
                  int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка при открытии файла" << std::endl;
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
    auto startTime = omp_get_wtime();
    double cReal = 0.0;
    double cImaginary = 0.0;
    double zr = 0.0;
    double zi = 0.0;
#pragma omp parallel for collapse(2)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            // Перевод координаты пикселя в комплексную плоскость
            cReal = MIN_REAL + x * (MAX_REAL - MIN_REAL) / WIDTH;
            cImaginary =
                MAX_IMAGINARY - y * (MAX_IMAGINARY - MIN_IMAGINARY) / HEIGHT;

            zr = 0.0;
            zi = 0.0;

            int iterations = 0;
            while (zr * zr + zi * zi <= 4.0 && iterations < MAX_ITERATIONS) {
                double temp = zr * zr - zi * zi + cReal;
                zi = 2.0 * zr * zi + cImaginary;
                zr = temp;
                iterations++;
            }

            int index = y * WIDTH + x;
            colors[index] = setColor(iterations);
        }
    }
    auto endTime = omp_get_wtime();
    std::cout << "Время: " << endTime - startTime << "\n";
    saveImagePPM("mandelbrot_openmp.ppm", colors, WIDTH, HEIGHT);

    return 0;
}
