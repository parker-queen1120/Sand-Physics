#pragma once
#include <GL/glew.h>

GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSrc, const char* fragmentSrc);
void useProgram(GLuint program);