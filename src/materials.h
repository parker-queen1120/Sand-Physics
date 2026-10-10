#pragma once

// material movement types 
enum class MovementType {
    Static,
    Powder,
    Liquid,
    Gas,
};

// material information structure
struct MaterialInfo {
    unsigned char r, g, b;
    MovementType movement;
    int diagonal_enabled;
};

// index = material id (materials[0] == air)
static const MaterialInfo materials[] = {
    {20, 20, 26, MovementType::Static, 0}, // most likely switch to gas later
    {235, 200, 140, MovementType::Powder, 1}, // sand
    {100, 150, 255, MovementType::Liquid, 1}, // water
    {255, 255, 255, MovementType::Powder, 0}, // wet sand
};

// constexpr for the number of materials
constexpr int materialCount = sizeof(materials) / sizeof(materials[0]);