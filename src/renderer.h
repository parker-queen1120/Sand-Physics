#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Renderer {
    GLuint vao, vbo;
    GLuint texture;
    GLuint shader;
    int gridCols, gridRows;

    void init(int width, int height, int gridCols, int gridRows);
    void draw(const std::vector<std::vector<int>>& cellArray, const glm::mat4& projection);
    void cleanup();
};