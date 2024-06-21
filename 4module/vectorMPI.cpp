#include <mpi.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

constexpr int ARRAY_SIZE = 9090908;
const int ROOT = 0;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int worldSize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> numbers(ARRAY_SIZE);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(1, ARRAY_SIZE);

    for (auto& number : numbers) {
        number = distribution(gen);
    }
    std::size_t localSize = ARRAY_SIZE / worldSize;
    std::vector<int> localNumbers(localSize);

    MPI_Scatter(numbers.data(), localSize, MPI_INT, localNumbers.data(),
                localSize, MPI_INT, ROOT, MPI_COMM_WORLD);

    std::sort(localNumbers.begin(), localNumbers.end());

    MPI_Gather(localNumbers.data(), localSize, MPI_INT, numbers.data(),
               localSize, MPI_INT, ROOT, MPI_COMM_WORLD);

    if (rank == ROOT) {
        for (int i = 1; i < worldSize; i++) {
            std::inplace_merge(numbers.begin(), numbers.begin() + i * localSize,
                               numbers.begin() + (i + 1) * localSize);
        }

        if (std::is_sorted(numbers.begin(), numbers.end())) {
            std::cout << "Список отсортирован верно\n";
        } else {
            std::cerr << "Список отсортирован неверно\n";
        }
    }

    MPI_Finalize();

    return 0;
}