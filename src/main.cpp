#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "simulation.h"
#include "renderer.h"
#include "materials.h"

bool showGrid = false;
static int selectedMaterial = 1;

// window resize function
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// keypress check function
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        // enable/disable grid
        showGrid = !showGrid;
    }
    // material swap with [/] keys
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

    // needed for alpha blending like on the grid
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // grid config
    const int cellSize = 8;
    const int gridSize = 30;
    int gridCols = width / cellSize;
    int gridRows = height / cellSize;

    // initialize the simulation and renderer
    Simulation simulation;
    simulation.init(gridCols, gridRows);
    Renderer renderer;
    renderer.init(width, height, gridCols, gridRows);

    // projection matrix for the renderer
    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    // frame consistency for the simulation loop
    double lastTime = glfwGetTime();

    // main loop for the simulation and rendering
    while (!glfwWindowShouldClose(window)) {
        // clear the screen with the background color
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // framestepping for consistency
        double now = glfwGetTime();
        double deltaTime = now - lastTime;
        lastTime = now;

        // mouse handling
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            int col = (int)(mouseX / cellSize);
            int row = (int)(mouseY / cellSize);
            simulation.placeAt(row, col, selectedMaterial);
            std::cout << "Placed material " << selectedMaterial << " at (" << row << ", " << col << ")\n";
        }

        // call the renderer and simulation to step
        renderer.draw(simulation.cellArray, projection, glm::vec2((float)mouseX, (float)mouseY), gridSize, cellSize);
        simulation.update(deltaTime);

        // display update
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer.cleanup();
    glfwTerminate();
    return 0;
}
