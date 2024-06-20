#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <complex>
#include <omp.h>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;
constexpr int MAX_ITERATIONS = 100;
constexpr double EPSILON = 1e-6;

int newtonMethod(const std::complex<double>& z0) {
    std::complex<double> z = z0;
    for (int i = 1; i <= MAX_ITERATIONS; ++i) {
        z = z - (z * z * z - 1.0) / (3.0 * z * z);
        if (std::abs(z * z * z - 1.0) < EPSILON) {
            return i;
        }
    }
    return -1; 
}

void saveImagePPM(const std::string& filename, const std::vector<int>& pixels, int width, int height) {
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
    std::vector<int> pixels(WIDTH * HEIGHT);

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double x0 = (x - WIDTH / 2.0) / (WIDTH / 4.0);
            double y0 = (y - HEIGHT / 2.0) / (HEIGHT / 4.0);
            std::complex<double> z(x0, y0);

            int iterations = newtonMethod(z);
            int index = y * WIDTH + x;
            pixels[index] = iterations;
        }
    }

    saveImagePPM("newton_openmp.ppm", pixels, WIDTH, HEIGHT);

    return 0;
}