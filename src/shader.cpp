#include "shader.h"
#include <iostream>

// shader compilation function
GLuint compileShader(GLenum type, const char* source) {
    // create a shader of the given type
    GLuint shader = glCreateShader(type);
    // stuff it into a variable
    glShaderSource(shader, 1, &source, nullptr);
    // compile it
    glCompileShader(shader);

    // error checking for shader compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << "\n";
    }
    return shader;
}

// shader program creation function
GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    // compiles a vertex and frag shader
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    // creates a shader program, adds the new vert and frag shader to it
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // error check
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader link error: " << infoLog << "\n";
    }

    // delete the individual shaders as they are no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}
// default to no program being used
static GLuint currentProgram = 0;

// helper to change program in time with sim steps
void useProgram(GLuint program) {
    if (program != currentProgram) {
        glUseProgram(program);
        currentProgram = program;
    }
}