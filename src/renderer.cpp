#include "renderer.h"
#include "shader.h"
#include "materials.h"

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
    FragColor = vec4(texture(uGridTexture, vTexCoord).rgb, 1.0);
}
)";

static const char* gridVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uProjection;
out vec2 vPos;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vPos = aPos;
}
)";

static const char* gridFragmentShaderSrc = R"(
#version 330 core
in vec2 vPos;
out vec4 FragColor;

uniform vec2 uCursorPos;
uniform float uRadius;
uniform float uCellSize;

void main() {
    vec2 gridPos = mod(vPos, uCellSize);
    bool onLine = gridPos.x < 1.0 || gridPos.y < 1.0;

    float dist = distance(vPos, uCursorPos);
    float fade = 1.0 - smoothstep(uRadius * 0.5, uRadius, dist);

    if (!onLine) discard;

    float maxAlpha = 0.8;
    FragColor = vec4(.7, .7, .7, fade * maxAlpha);
}
)";


static void updateGridTexture(GLuint texture, const std::vector<std::vector<int>>& cellArray, int gridCols, int gridRows) {
    std::vector<unsigned char> gridPixels(gridCols * gridRows * 3);
    
    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {
            const MaterialInfo& mat = materials[cellArray[row][col]];
            int idx = (row * gridCols + col) * 3;
            gridPixels[idx] = (unsigned char)(mat.r);
            gridPixels[idx + 1] = (unsigned char)(mat.g);
            gridPixels[idx + 2] = (unsigned char)(mat.b);
            
            }
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridCols, gridRows, GL_RGB, GL_UNSIGNED_BYTE, gridPixels.data());
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gridCols, gridRows, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

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
    gridShader = createShaderProgram(gridVertexShaderSrc, gridFragmentShaderSrc);
}

void Renderer::draw(const std::vector<std::vector<int>>& cellArray, const glm::mat4& projection, glm::vec2 cursorPos, int gridRadius, int cellSize) {
    updateGridTexture(texture, cellArray, gridCols, gridRows);

    useProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "uGridTexture"), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    useProgram(gridShader);
    glUniformMatrix4fv(glGetUniformLocation(gridShader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform2f(glGetUniformLocation(gridShader, "uCursorPos"), cursorPos.x, cursorPos.y);
    glUniform1f(glGetUniformLocation(gridShader, "uRadius"), (float)gridRadius);
    glUniform1f(glGetUniformLocation(gridShader, "uCellSize"), (float)cellSize);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
    glDeleteProgram(gridShader);
}