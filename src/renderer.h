#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Renderer {
    GLuint vao, vbo;
    GLuint texture;
    GLuint shader, gridShader;
    int gridCols, gridRows;

    void init(int width, int height, int gridCols, int gridRows);
    void draw(const std::vector<std::vector<int>>& cellArray, const glm::mat4& projection, glm::vec2 cursorPos, int gridRadius, int cellSize);
    void cleanup();
};