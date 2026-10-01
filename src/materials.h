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
};

// index = material id (materials[0] == air)
static const MaterialInfo materials[] = {
    {20, 20, 26, MovementType::Static}, // most likely switch to gas later
    {235, 200, 140, MovementType::Powder},
    {100, 150, 255, MovementType::Liquid},
};

// constexpr for the number of materials
constexpr int materialCount = sizeof(materials) / sizeof(materials[0]);