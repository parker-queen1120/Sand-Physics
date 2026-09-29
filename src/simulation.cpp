#include "simulation.h"
#include <utility>

void Simulation::init(int cols, int rows) {
    gridCols = cols;
    gridRows = rows;
    
    for (int i = 0; i < gridRows; ++i) {
        cellArray.emplace_back();
        velocityArray.emplace_back();
        for (int j = 0; j < gridCols; ++j) {
            cellArray.back().push_back(0);
            velocityArray.back().push_back(0);
        }
    }
}

void Simulation::placeAt(int row, int col) {
    if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
        cellArray[row][col] = 1;
        velocityArray[row][col] = 0;
    }
}

void Simulation::update(double deltaTime) {
    accumulator += deltaTime;
    while (accumulator >= simStep) {
        step();
        accumulator -= simStep;
    }
}

void Simulation::step() {
    std::vector<std::vector<int>> bufferArray(gridRows, std::vector<int>(gridCols, 0));
    std::vector<std::vector<int>> bufferVelocity(gridRows, std::vector<int>(gridCols, 0));
    
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            if (cellArray[i][j] == 1) {
                int v = velocityArray[i][j] + 1;
                int row = i;
                int steps = 0;
                while (steps < v && row + 1 < gridRows && cellArray[row+1][j] == 0 && bufferArray[row+1][j] == 0) {
                    row++;
                    steps++;
                }

                if (steps > 0) {
                    bufferArray[row][j] = 1;
                    bufferVelocity[row][j] = v;
                } else {
                    if (i + 1 < gridRows && j + 1 < gridCols && cellArray[i+1][j+1] == 0 && bufferArray[i+1][j+1] == 0) {
                        bufferArray[i+1][j+1] = 1;
                        bufferVelocity[i+1][j+1] = 0;
                    } else if (i + 1 < gridRows && j - 1 >= 0 && cellArray[i+1][j-1] == 0 && bufferArray[i+1][j-1] == 0) {
                        bufferArray[i+1][j-1] = 1;
                        bufferVelocity[i+1][j-1] = 0;
                    } else {
                        bufferArray[i][j] = 1;
                        bufferVelocity[i][j] = 0;
                    }
                }
            }
        }
    }
    //swap the buffers
    cellArray = std::move(bufferArray);
    velocityArray = std::move(bufferVelocity);
}