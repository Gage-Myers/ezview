/*--------------------------------------
CS 430 - Project 5: Image Viewer
File: ezview.c
Author: Gage Myers
-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "ppmrw.h"
#include "matrix.h"

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>
#include <GLES2/gl2.h>

#define PI = acos(-1.0)
#define GLFW_DLL 1
#define GL_GLEXT_PROTOTYPES

// Structure for doing affine transforms
typedef struct {
  float Position[2];
  float TexCoord[2];
} vertex;

typedef struct {
  float scale;
  float rotate;
  float translate[2]; // 0 -> x, 1 -> y
  float shear[2];     // 0 -> x, 1 -> y
} affine; /* Contains properties of the image */

int img_width, img_height, max_color, ppm_ver;
pixel      *buffer;
affine *affine_mat;
GLuint     vBuffer, vShader, fShader, program;
GLint      mvpLocation, vPosLocation, vColLocation;
GLFWwindow* window;

vertex     vertices[] = {
            { {  1  , -1  }  ,  { 0.99999, 0.99999 } },
            { {  1  ,  1  }  ,  { 0.99999, 0.00000 } },
            { { -1  ,  1  }  ,  { 0.00000, 0.00000 } },
            { { -1  ,  1  }  ,  { 0.00000, 0.00000 } },
            { { -1  , -1  }  ,  { 0.00000, 0.99999 } },
            { {  1  , -1  }  ,  { 0.99999, 0.99999 } }
};


static const GLchar* vertex_shader_source =
    "uniform mat4 MVP;\n"
    "attribute vec2 TexCoordIn;\n"
    "attribute vec2 vPos;\n"
    "varying vec2 TexCoordOut;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    TexCoordOut = TexCoordIn;\n"
    "}\n";
static const GLchar* fragment_shader_source =
    "varying lowp vec2 TexCoordOut;\n"
    "uniform sampler2D Texture;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
    "}\n";


static void error_callback(int error, const char* description) {
     fprintf(stderr, "Error: %s\n", description);
}

static inline void compileShader(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s.\n", info);
    exit(1);
  }
}

static void key_event_handler( GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Scale Increase
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      affine_mat[0].scale *= 1.1;

    // Scale Decrease
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    	affine_mat[0].scale /= 1.1;

    // Translate Right
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    	affine_mat[0].translate[0] += 0.1;

    // Translate Left
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    	affine_mat[0].translate[0] -= 0.1;

    // Translate Up
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
      affine_mat[0].translate[1] += 0.1;

    // Translate Down
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
      affine_mat[0].translate[1] -= 0.1;

    // Shear Up
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    	affine_mat[0].shear[1] += 0.1;

    // Shear Down
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    	affine_mat[0].shear[1] -= 0.1;

    // Shear Right
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    	affine_mat[0].shear[0] += 0.1;

    // Shear Left
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    	affine_mat[0].shear[0] -= 0.1;

    // Rotate Counter-Clockwise
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
      affine_mat[0].rotate += (90 * M_PI) / 180;

    // Rotate Clockwise
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
      affine_mat[0].rotate -= (90 * M_PI) / 180;
}

int init() {
    GLuint shader_program, vbo;
    GLint pos;

    /* Create Error Callback */
    glfwSetErrorCallback(error_callback);

    /* Initialize GLFW */
    if (!glfwInit()) {
    fprintf(stderr, "Could not initialize GLFW.\n");
    exit(1);
    }

    /* Set Window Version */
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    /* Create Window */
    window = glfwCreateWindow(800, 600, "Image Viewer", NULL, NULL);
    if (!window) {
    fprintf(stderr, "Could not initialize window.\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
    }

    /* Set Key Event Callback */
    glfwSetKeyCallback(window, key_event_handler);

    /* Construct */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertex_shader_source, NULL);
    compileShader(vShader);

    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragment_shader_source, NULL);
    compileShader(fShader);

    program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    mvpLocation = glGetUniformLocation(program, "MVP");
    assert(mvpLocation != -1);

    vPosLocation = glGetAttribLocation(program, "vPos");
    assert(vPosLocation != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vPosLocation);
    glVertexAttribPointer(vPosLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void*) 0);
    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void*) (sizeof(float) * 2));
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, buffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);
    return 1;
}

/*
 * Runs the program after initialization
 */
void run() {

  /* Initialize the Affine Transformations */
  affine_mat = (affine *) malloc(sizeof(affine));
  affine_mat[0].translate[0] = 0.0;
  affine_mat[0].translate[1] = 0.0;
  affine_mat[0].shear[0]     = 0.0;
  affine_mat[0].shear[1]     = 0.0;
  affine_mat[0].rotate       = 0.0;
  affine_mat[0].scale        = 1.0;

  /* Run the Program */
  while (!glfwWindowShouldClose(window)) {
    float ratio;
    int imgWidth, imgHeight;
    mat4x4 rotate, shear, scale, translate, rs, rss, affMat;
    glfwGetFramebufferSize(window, &imgWidth, &imgHeight);
    ratio = imgWidth / (float) imgHeight;

    glViewport(0, 0, imgWidth, imgHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Rotate the matrix */
    mat4x4_identity(rotate);
    mat4x4_rotate_Z(rotate, rotate, affine_mat[0].rotate);

    /* Shear the matrix */
    mat4x4_identity(shear);
    shear[1][0] = affine_mat[0].shear[0];
    shear[0][1] = affine_mat[0].shear[1];

    /* Scale the matrix */
    mat4x4_identity(scale);
    scale[0][0] = scale[0][0] * affine_mat[0].scale;
    scale[1][1] = scale[1][1] * affine_mat[0].scale;

    /* Translate the matrix */
    mat4x4_identity(translate);
    mat4x4_translate(translate, affine_mat[0].translate[0], affine_mat[0].translate[1], 0);

    /* Total the transformations */
    mat4x4_mul(rs, rotate, shear);
    mat4x4_mul(rss, rs, scale);
    mat4x4_mul(affMat, rss, translate);

    /* Update the Program */
    glUseProgram(program);
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat*) affMat);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* Send the window to the front and poll */
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(0);
}


int main(int argc, char *argv[])  {
    /* Verify that correct number of arguments are supplied */
  if (argc != 2) {
    fprintf(stderr, "\nERROR: Incorrect number of arguments."
                    "\nExpected two arguments, %d were supplied."
                    "\nFormat: Program_Name File_Name.", argc);
    exit(1);
  }

  /* Open Image and read-in properties */
  FILE *img_file;
  img_file = fopen(argv[1], "rb");
  int error = initiate(img_file, &img_width, &img_height, &max_color, &ppm_ver);
  if (error) {
    switch(error) {
      /* Case - Invalid Magic Number */
      case 1:
        fprintf(stderr, "\nERROR %d: Invalid or no magic number detected"
                        " in file.", error);
        break;

      /* Case - File supplied for input does not exist */
      case 2:
        fprintf(stderr, "\nERROR %d: Input file does not exist.",
            error);
        break;

      /* Case - Invalid Header */
      case 3:
        fprintf(stderr, "\nERROR %d: Header is invalid.", error);
        break;
    }
    fclose(img_file);
    exit(1);
  }

  /* Read-in image to pixel buffer */
  buffer = (pixel *) malloc(sizeof(pixel) * img_width * img_height);

  if (ppm_ver == 3) {
    read3(img_file, buffer, &img_width, &img_height, &max_color);
  } else {
    read6(img_file, buffer, &img_width, &img_height, &max_color);
  }

  fclose(img_file);
  /** Create Window */
  if (!init()) {
    fprintf(stderr, "Could not create window.\n");
    exit(1);
  } else {
    run();
  }
}
