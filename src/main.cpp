#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static const char* gridVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

static const char* gridFragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 uColor;

void main() {
    FragColor = uColor;
}
)";

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
static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << "\n";
    }
    return shader;
}

static GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader link error: " << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

static GLuint currentProgram = 0;

static void useProgram(GLuint program) {
    if (program != currentProgram) {
        glUseProgram(program);
        currentProgram = program;
    }
}
// Builds the line-segment vertices for a grid of `cellSize`-pixel cells
// covering a `width` x `height` pixel area. Two floats (x, y) per vertex,
// two vertices per line segment.
static std::vector<float> buildGridLines(int width, int height, int cellSize) {
    std::vector<float> vertices;

    for (int x = 0; x <= width; x += cellSize) {
        vertices.insert(vertices.end(), { (float)x, 0.0f, (float)x, (float)height });
    }
    for (int y = 0; y <= height; y += cellSize) {
        vertices.insert(vertices.end(), { 0.0f, (float)y, (float)width, (float)y });
    }

    return vertices;
}

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

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on macOS

    int width = 1280;
    int height = 720;
    GLFWwindow* window = glfwCreateWindow(width, height, "Sand Physics", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus) << "\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    const int cellSize = 40;
    int gridCols = width / cellSize;
    int gridRows = height / cellSize;

    std::vector<float> gridVertices = buildGridLines(width, height, cellSize);
    GLsizei gridVertexCount = (GLsizei)(gridVertices.size() / 2); // 2 floats per vertex

    std::vector<std::vector<int>> cellArray;

    for (int i = 0; i < gridRows; ++i) {
        cellArray.emplace_back();
        for (int j = 0; j < gridCols; ++j) {
            cellArray.back().push_back(0);
        }
    }
    cellArray[0][gridCols / 2] = 1;

    GLuint gridTexture;
    glGenTextures(1, &gridTexture);
    glBindTexture(GL_TEXTURE_2D, gridTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::vector<float> gridQuadVertices = {
        //    x,           y,          u,   v
        0.0f,         0.0f,          0.0f, 0.0f,
        (float)width, 0.0f,          1.0f, 0.0f,
        (float)width, (float)height, 1.0f, 1.0f,
        0.0f,         (float)height, 0.0f, 1.0f,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gridCols, gridRows, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    
    updateGridTexture(gridTexture, cellArray, gridCols, gridRows);

    GLuint gridVAO, gridVBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    GLuint sandVAO, sandVBO;
    glGenVertexArrays(1, &sandVAO);
    glGenBuffers(1, &sandVBO);

    glBindVertexArray(sandVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sandVBO);
    glBufferData(GL_ARRAY_BUFFER, gridQuadVertices.size() * sizeof(float), gridQuadVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint gridShader = createShaderProgram(gridVertexShaderSrc, gridFragmentShaderSrc);
    GLuint sandShader = createShaderProgram(sandVertexShaderSrc, sandFragmentShaderSrc);

    // Pixel-space projection: (0,0) at top-left, (width,height) at bottom-right,
    // matching GLFW's window/cursor coordinate convention.
    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    GLint uProjectionLoc = glGetUniformLocation(gridShader, "uProjection");
    GLint uColorLoc = glGetUniformLocation(gridShader, "uColor");
    
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // use the grid shader
        useProgram(gridShader);
        glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, &projection[0][0]);
        glUniform4f(uColorLoc, 0.35f, 0.35f, 0.4f, 1.0f);

        // use the grid vao 
        glBindVertexArray(gridVAO);
        glDrawArrays(GL_LINES, 0, gridVertexCount);
        glBindVertexArray(0);

        // update the grid texture with the current cell array
        updateGridTexture(gridTexture, cellArray, gridCols, gridRows);

        // use the sand shader
        useProgram(sandShader);
        glUniformMatrix4fv(glGetUniformLocation(sandShader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform4f(glGetUniformLocation(sandShader, "uColor"), 1.0f, 0.5f, 0.2f, 1.0f);
        
        // use the sand vao
        glBindVertexArray(sandVAO);

        // bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gridTexture);
        glUniform1i(glGetUniformLocation(sandShader, "uGridTexture"), 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, gridQuadVertices.size() / 4);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &sandVAO);
    glDeleteBuffers(1, &sandVBO);
    glDeleteProgram(gridShader);
    glDeleteProgram(sandShader);
    glDeleteTextures(1, &gridTexture);

    glfwTerminate();
    return 0;
}
