#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
bool showGrid = false;

#include "simulation.h"
#include "renderer.h"

static int selectedMaterial = 1;
#include "materials.h"

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        showGrid = !showGrid;
    }
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
        selectedMaterial = (selectedMaterial + 1) % materialCount;
    }
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
        selectedMaterial = (selectedMaterial - 1 + materialCount) % materialCount;
    }
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int cellSize = 8;
    const int gridSize = 30;
    int gridCols = width / cellSize;
    int gridRows = height / cellSize;

    Simulation simulation;
    simulation.init(gridCols, gridRows);
    Renderer renderer;
    renderer.init(width, height, gridCols, gridRows);

    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        double now = glfwGetTime();
        double deltaTime = now - lastTime;
        lastTime = now;

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            int col = (int)(mouseX / cellSize);
            int row = (int)(mouseY / cellSize);
            simulation.placeAt(row, col, selectedMaterial);
            std::cout << "Placed material " << selectedMaterial << " at (" << row << ", " << col << ")\n";
        }

        renderer.draw(simulation.cellArray, projection, glm::vec2((float)mouseX, (float)mouseY), gridSize, cellSize);
        simulation.update(deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer.cleanup();
    glfwTerminate();
    return 0;
}
