#include "simulation.h"
#include "materials.h"
#include <utility>

// simulation initialization
void Simulation::init(int cols, int rows) {
    gridCols = cols;
    gridRows = rows;
    
    // initialize 2 empty 2d matrices, the array for what blocks are where, and a matching one for their vertical velocities
    for (int i = 0; i < gridRows; ++i) {
        cellArray.emplace_back();
        velocityArray.emplace_back();
        for (int j = 0; j < gridCols; ++j) {
            cellArray.back().push_back(0);
            velocityArray.back().push_back(0);
        }
    }
}

// function to place a material in the simulation grid
void Simulation::placeAt(int row, int col, int materialId) {
    if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
        // places RIGHT into the cellArray instead of the buffer
        cellArray[row][col] = materialId;
        velocityArray[row][col] = 0;
    }
}

// handles simulation updates; accounts for acceleration accumulation.
void Simulation::update(double deltaTime) {
    accumulator += deltaTime;
    while (accumulator >= simStep) {
        step();
        accumulator -= simStep;
    }
}

// function for materials typed as "Powder" and how they move
void Simulation::stepPowder(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity) {
    // increase the velocity each frame
    int v = velocityArray[row][col] + 1;

    // temp variables for multi-step checks
    int row_ = row;
    int col_ = col;

    // step counting
    int steps = 0;

    // determine which cell to check based off of velocity (predict how far the block is trying to fall)
    while (steps < v && row_ + 1 < gridRows && cellArray[row_+1][col] == 0 && bufferArray[row_+1][col] == 0) {
        row_++;
        steps++;
    }

    // if the block has moved, place it in the buffer at the predicted position
    if (steps > 0) {
        bufferArray[row_][col_] = id;
        bufferVelocity[row_][col_] = v;
    } else {
        // if the block did not move downward, try to move diagonally instead
        if (row_ + 1 < gridRows && col_ + 1 < gridCols && cellArray[row_+1][col_+1] == 0 && bufferArray[row_+1][col_+1] == 0) {
            bufferArray[row_+1][col_+1] = id;
            bufferVelocity[row_+1][col_+1] = 0;
        } else if (row_ + 1 < gridRows && col_ - 1 >= 0 && cellArray[row_+1][col_-1] == 0 && bufferArray[row_+1][col_-1] == 0) {
            bufferArray[row_+1][col_-1] = id;
            bufferVelocity[row_+1][col_-1] = 0;
        } else {
            // or stay put
            bufferArray[row_][col_] = id;
            bufferVelocity[row_][col_] = 0;
        }
    }
}

// simstep for materials typed as "Liquid"
void Simulation::stepLiquid(int row, int col, int id, std::vector<std::vector<int>>& bufferArray, std::vector<std::vector<int>>& bufferVelocity) {
    // magic number how how many times per frame a liquid will try to horizontally spread
    const int spreadRate = 3;

    // increase falling velocity per frame
    int v = velocityArray[row][col] + 1;

    // temp variables for multi-step checks
    int row_ = row;
    int col_ = col;

    // step tracking
    int steps = 0;
    
    // attempt to fall based on velocity
    while (steps < v && row_+1 < gridRows && cellArray[row_+1][col] == 0 && bufferArray[row_+1][col] == 0) {
        row_++; steps++;
    }

    // fall if you can
    if (steps > 0) {
        bufferArray[row_][col] = id;
        bufferVelocity[row_][col] = v;
        return;
    }

    // if the pixel is not able to fall, try diagonal
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

    //  try spreadRate times to spread in a random direction
    for (int i = 0; i < spreadRate; ++i) {
        int dir = coinFlip(rng) == 0 ? -1 : 1;
        int nextCol = col_ + dir;
        if (nextCol >= 0 && nextCol < gridCols && cellArray[row][nextCol] == 0 && bufferArray[row][nextCol] == 0) {
            col_ = nextCol;
        }
    }

    // if the liquid reached a new spot in the attempted moves, move
    if (col_ != col) {
        bufferArray[row][col_] = id;
        bufferVelocity[row][col_] = 0;
        return;
    }

    // if the liquid didn't move, place it back in the original position
    bufferArray[row][col] = id;
    bufferVelocity[row][col] = 0;
}

// actual step handling
void Simulation::step() {
    std::vector<std::vector<int>> bufferArray(gridRows, std::vector<int>(gridCols, 0));
    std::vector<std::vector<int>> bufferVelocity(gridRows, std::vector<int>(gridCols, 0));
    
    // cell iteration
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            // get material id of current cell
            int id = cellArray[i][j];
            // dont update if its air (will be changed if i change air to a gas)
            if (id == 0) continue;

            // determine which movement function needs to be used
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