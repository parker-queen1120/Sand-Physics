#include "renderer.h"
#include "shader.h"

// vert shader for the sand
static const char* sandVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
out vec2 vTexCoord;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

// fragment shader for the sand
static const char* sandFragmentShaderSrc = R"(
#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uGridTexture;

void main() {
    float cell = texture(uGridTexture, vTexCoord).r;
    FragColor = mix(vec4(0.08, 0.08, 0.1, 1.0), vec4(1.0, 0.8, 0.4, 1.0), cell);
}
)";

static void updateGridTexture(GLuint texture, const std::vector<std::vector<int>>& cellArray, int gridCols, int gridRows) {
    std::vector<unsigned char> gridPixels(gridCols * gridRows);
    
    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {
            gridPixels[row * gridCols + col] = (unsigned char)(cellArray[row][col] * 255);
        }
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridCols, gridRows, GL_RED, GL_UNSIGNED_BYTE, gridPixels.data());
}

void Renderer::init(int width, int height, int cols, int rows) {
    gridCols = cols;
    gridRows = rows;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gridCols, gridRows, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    std::vector<float> gridQuadVertices = {
        //    x,           y,          u,   v
        0.0f,         0.0f,          0.0f, 0.0f,
        (float)width, 0.0f,          1.0f, 0.0f,
        (float)width, (float)height, 1.0f, 1.0f,
        0.0f,         (float)height, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, gridQuadVertices.size() * sizeof(float), gridQuadVertices.data(), GL_DYNAMIC_DRAW);

    // attribute for position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // attribute for texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    shader = createShaderProgram(sandVertexShaderSrc, sandFragmentShaderSrc);
}

void Renderer::draw(const std::vector<std::vector<int>>& cellArray, const glm::mat4& projection) {
    updateGridTexture(texture, cellArray, gridCols, gridRows);

    useProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4f(glGetUniformLocation(shader, "uColor"), 1.0f, 0.5f, 0.2f, 1.0f);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "uGridTexture"), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
}