#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// Callback for when the framebuffer size changes
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback for when a key is pressed or released
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

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

// shader compilation utility
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

// shader program linker
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

// for program switching in the loop; cheap to call
static GLuint currentProgram = 0;

static void useProgram(GLuint program) {
    if (program != currentProgram) {
        glUseProgram(program);
        currentProgram = program;
    }
}

// updates the texture so we can draw all the sand at once
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
    // error handling for GLFW initialization
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // configure OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on macOS

    // set the window dimensions
    int width = 1280;
    int height = 720;
    GLFWwindow* window = glfwCreateWindow(width, height, "Sand Physics", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // make the OpenGL context current and set up callbacks
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // initialize GLEW to get access to modern OpenGL functions
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus) << "\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // set up the grid and texture for rendering the sand
    const int cellSize = 5;
    int gridCols = width / cellSize;
    int gridRows = height / cellSize;

    std::vector<std::vector<int>> cellArray;
    std::vector<std::vector<int>> velocityArray;

    for (int i = 0; i < gridRows; ++i) {
        cellArray.emplace_back();
        velocityArray.emplace_back();
        for (int j = 0; j < gridCols; ++j) {
            cellArray.back().push_back(0);
            velocityArray.back().push_back(0);
        }
    }

    // initialize one cell to filled
    cellArray[0][gridCols / 2] = 1;
    velocityArray[0][gridCols / 2] = 0;
    // set up the texture for rendering the sand
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
    
    // initialize the texture with empty sand data
    updateGridTexture(gridTexture, cellArray, gridCols, gridRows);

    // set up the sand VAO and VBO for rendering the sand texture
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

    // create the shader program for the sand
    GLuint sandShader = createShaderProgram(sandVertexShaderSrc, sandFragmentShaderSrc);

    // set up the projection matrix since we are rendering in pixel space
    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    
    // time keeping for simulation speed
    double lastTime = glfwGetTime();
    double accumulator = 0.0;
    const double simStep = 1.0 / 20.0;

    // main rendering loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        double now = glfwGetTime();
        accumulator += now - lastTime;
        lastTime = now;

        // drag to place sand logic
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            int col = (int)(mouseX / cellSize);
            int row = (int)(mouseY / cellSize);
            if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
                cellArray[row][col] = 1;
                velocityArray[row][col] = 0;
            }
        }

        // update the sand texture with the current cell array
        updateGridTexture(gridTexture, cellArray, gridCols, gridRows);

        // render the sand
        useProgram(sandShader);
        glUniformMatrix4fv(glGetUniformLocation(sandShader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform4f(glGetUniformLocation(sandShader, "uColor"), 1.0f, 0.5f, 0.2f, 1.0f);
        glBindVertexArray(sandVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gridTexture);
        glUniform1i(glGetUniformLocation(sandShader, "uGridTexture"), 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, gridQuadVertices.size() / 4);
        glBindVertexArray(0);

        // sand update logic
        while (accumulator >= simStep) {
            std::vector<std::vector<int>> bufferArray(gridRows, std::vector<int>(gridCols, 0));
            std::vector<std::vector<int>> bufferVelocity(gridRows, std::vector<int>(gridCols, 0));
            
            for (int i = 0; i < gridRows; i++) {
                for (int j = 0; j < gridCols; j++) {
                    if (cellArray[i][j] == 1) {
                        int v = velocityArray[i][j] + 1;
                        int row = i;
                        int steps = 0;
                        while (steps < v && row + 1 < gridRows && cellArray[row+1][j] == 0 && bufferArray[row+1][j] == 0) {
                            row++;
                            steps++;
                        }

                        if (steps > 0) {
                            bufferArray[row][j] = 1;
                            bufferVelocity[row][j] = v;
                        } else {
                            if (i + 1 < gridRows && j + 1 < gridCols && cellArray[i+1][j+1] == 0 && bufferArray[i+1][j+1] == 0) {
                                bufferArray[i+1][j+1] = 1;
                                bufferVelocity[i+1][j+1] = 0;
                            } else if (i + 1 < gridRows && j - 1 >= 0 && cellArray[i+1][j-1] == 0 && bufferArray[i+1][j-1] == 0) {
                                bufferArray[i+1][j-1] = 1;
                                bufferVelocity[i+1][j-1] = 0;
                            } else {
                                bufferArray[i][j] = 1;
                                bufferVelocity[i][j] = 0;
                            }
                            

                        }
                    }
                }
            }

            //swap the buffers
            cellArray = std::move(bufferArray);
            velocityArray = std::move(bufferVelocity);

            accumulator -= simStep;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &sandVAO);
    glDeleteBuffers(1, &sandVBO);
    glDeleteProgram(sandShader);
    glDeleteTextures(1, &gridTexture);

    glfwTerminate();
    return 0;
}
