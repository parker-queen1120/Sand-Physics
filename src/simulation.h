#pragma once
#include <vector>

struct Simulation {
    std::vector<std::vector<int>> cellArray;
    std::vector<std::vector<int>> velocityArray;
    int gridCols, gridRows;

    void init(int gridCols, int gridRows);
    void update(double deltaTime);
    void placeAt(int row, int col);

private:
    double accumulator = 0.0;
    const double simStep = 1.0 / 20.0;

    void step();
};