#include <mpi.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

constexpr int ARRAY_SIZE = 9090908;

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

    int localSize = ARRAY_SIZE / worldSize;
    std::vector<int> localNumbers(localSize);
    MPI_Scatter(numbers.data(), localSize, MPI_INT, localNumbers.data(),
                localSize, MPI_INT, 0, MPI_COMM_WORLD);

    std::sort(localNumbers.begin(), localNumbers.end());

    std::vector<int> sortedNumbers(ARRAY_SIZE);
    MPI_Gather(localNumbers.data(), localSize, MPI_INT, sortedNumbers.data(),
               localSize, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::sort(sortedNumbers.begin(), sortedNumbers.end());
    }

    MPI_Finalize();

    return 0;
}