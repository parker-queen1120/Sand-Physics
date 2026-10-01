#include "simulation.h"
#include "materials.h"
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

void Simulation::placeAt(int row, int col, int materialId) {
    if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
        cellArray[row][col] = materialId;
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

void Simulation::stepPowder(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity) {
    int v = velocityArray[row][col] + 1;
    int row_ = row;
    int col_ = col;
    int steps = 0;
    while (steps < v && row_ + 1 < gridRows && cellArray[row_+1][col] == 0 && bufferArray[row_+1][col] == 0) {
        row_++;
        steps++;
    }

    if (steps > 0) {
        bufferArray[row_][col_] = id;
        bufferVelocity[row_][col_] = v;
    } else {
        if (row_ + 1 < gridRows && col_ + 1 < gridCols && cellArray[row_+1][col_+1] == 0 && bufferArray[row_+1][col_+1] == 0) {
            bufferArray[row_+1][col_+1] = id;
            bufferVelocity[row_+1][col_+1] = 0;
        } else if (row_ + 1 < gridRows && col_ - 1 >= 0 && cellArray[row_+1][col_-1] == 0 && bufferArray[row_+1][col_-1] == 0) {
            bufferArray[row_+1][col_-1] = id;
            bufferVelocity[row_+1][col_-1] = 0;
        } else {
            bufferArray[row_][col_] = id;
            bufferVelocity[row_][col_] = 0;
        }
    }
}

void Simulation::stepLiquid(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity) {
    const int spreadRate = 3;

    // 1. try falling straight down (accelerating, same as powder)
    int v = velocityArray[row][col] + 1;
    int row_ = row;
    int steps = 0;
    while (steps < v && row_+1 < gridRows && cellArray[row_+1][col] == 0 && bufferArray[row_+1][col] == 0) {
        row_++; steps++;
    }
    if (steps > 0) {
        bufferArray[row_][col] = id;
        bufferVelocity[row_][col] = v;
        return;
    }

    // 2. try diagonal fall
    if (row+1 < gridRows && col+1 < gridCols && cellArray[row+1][col+1] == 0 && bufferArray[row+1][col+1] == 0) {
        bufferArray[row+1][col+1] = id;
        bufferVelocity[row+1][col+1] = 0;
        return;
    }
    if (row+1 < gridRows && col-1 >= 0 && cellArray[row+1][col-1] == 0 && bufferArray[row+1][col-1] == 0) {
        bufferArray[row+1][col-1] = id;
        bufferVelocity[row+1][col-1] = 0;
        return;
    }

    // 3. try spreading sideways — independent checks, not shared state
    int col_ = col;
    for (int i = 0; i < spreadRate; ++i) {
        int dir = coinFlip(rng) == 0 ? -1 : 1;
        int nextCol = col_ + dir;
        if (nextCol >= 0 && nextCol < gridCols && cellArray[row][nextCol] == 0 && bufferArray[row][nextCol] == 0) {
            col_ = nextCol;
        }
    }
    if (col_ != col) {
        bufferArray[row][col_] = id;
        bufferVelocity[row][col_] = 0;
        return;
    }

    // 4. fully stuck
    bufferArray[row][col] = id;
    bufferVelocity[row][col] = 0;
}


void Simulation::step() {
    std::vector<std::vector<int>> bufferArray(gridRows, std::vector<int>(gridCols, 0));
    std::vector<std::vector<int>> bufferVelocity(gridRows, std::vector<int>(gridCols, 0));
    
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            int id = cellArray[i][j];
            if (id == 0) continue;

            switch (materials[id].movement) {
                case MovementType::Powder:
                    stepPowder(i, j, id, bufferArray, bufferVelocity);
                    break;
                case MovementType::Liquid:
                    stepLiquid(i, j, id, bufferArray, bufferVelocity);
                    break;
            }
        }
    }
    //swap the buffers
    cellArray = std::move(bufferArray);
    velocityArray = std::move(bufferVelocity);
}