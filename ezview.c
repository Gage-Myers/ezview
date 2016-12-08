/*--------------------------------------
CS 430 - Project 5: Image Viewer
File: ezview.c
Author: Gage Myers
-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;
static const GLchar* vertex_shader_source =
    "#version 100\n"
    "attribute vec3 position;\n"
    "void main() {\n"
    "   gl_Position = vec4(position, 1.0);\n"
    "}\n";
static const GLchar* fragment_shader_source =
    "#version 100\n"
    "void main() {\n"
    "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";
static const GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
};

/* A function that defines and retreaves the proper shader using our pre defined sources */
GLint common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) {
    enum Consts {INFOLOG_LEN = 512};
    GLchar infoLog[INFOLOG_LEN];
    GLint fragment_shader;
    GLint shader_program;
    GLint success;
    GLint vertex_shader;

    /* Vertex shader */
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Fragment shader */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Link shaders */
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}
