#pragma once

enum class MovementType {
    Static,
    Powder,
    Liquid,
    Gas,
};

struct MaterialInfo {
    unsigned char r, g, b;
    MovementType movement;
};

static const MaterialInfo materials[] = {
    {20, 20, 26, MovementType::Static},
    {235, 200, 140, MovementType::Powder},
    {100, 150, 255, MovementType::Liquid},
};

constexpr int materialCount = sizeof(materials) / sizeof(materials[0]);