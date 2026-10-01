#pragma once
#include <vector>
#include <random>

struct Simulation {
    std::vector<std::vector<int>> cellArray;
    std::vector<std::vector<int>> velocityArray;
    int gridCols, gridRows;

    void init(int gridCols, int gridRows);
    void update(double deltaTime);
    void placeAt(int row, int col, int materialId);

private:
    double accumulator = 0.0;
    const double simStep = 1.0 / 20.0;
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> coinFlip{0, 1};

    void step();
    void stepPowder(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity);
    void stepLiquid(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity);
};