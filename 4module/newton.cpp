#include <omp.h>

#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <vector>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;
constexpr int MAX_ITERATIONS = 100;
constexpr double EPSILON = 1e-6;
std::complex<double> roots[3] = {std::complex<double>(1.0, 0.0),
                                 std::complex<double>(-0.5, std::sqrt(3) / 2),
                                 std::complex<double>(-0.5, -std::sqrt(3) / 2)};
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

Color getColor(const std::complex<double>& z) {
    if (std::abs(z - roots[0]) < EPSILON) {
        return {0, 255, 0};
    }
    if (std::abs(z - roots[1]) < EPSILON) {
        return {255, 0, 0};
    }
    if (std::abs(z - roots[2]) < EPSILON) {
        return {0, 0, 255};
    }
    return {0, 0, 0};
}

int newtonMethod(const std::complex<double>& z0, Color& color) {
    std::complex<double> z = z0;
    for (int i = 1; i <= MAX_ITERATIONS; ++i) {
        z = z - (z * z * z - 1.0) / (3.0 * z * z);
        for (int j = 0; j < 3; j++) {
            if (std::abs(z - roots[j]) < EPSILON) {
                color = getColor(z);
                return i;
            }
        }
    }
    return -1;
}

void saveImagePPM(const std::string& filename, const std::vector<Color>& pixels,
                  int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка при открытии файла" << std::endl;
        return;
    }

    file << "P6\n";
    file << width << " " << height << "\n";
    file << "255\n";

    for (const auto& pixel : pixels) {
        file << pixel.r << pixel.g << pixel.b;
    }

    file.close();
}

int main() {
    std::vector<Color> pixels(WIDTH * HEIGHT);

#pragma omp parallel for collapse(2)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            Color color;
            double x0 = (x - WIDTH / 2.0) / (WIDTH / 4.0);
            double y0 = (y - HEIGHT / 2.0) / (HEIGHT / 4.0);
            std::complex<double> z(x0, y0);

            int iterations = newtonMethod(z, color);
            int index = y * WIDTH + x;
            pixels[index] = color;  // iterations;
        }
    }

    saveImagePPM("newton_openmp.ppm", pixels, WIDTH, HEIGHT);

    return 0;
}