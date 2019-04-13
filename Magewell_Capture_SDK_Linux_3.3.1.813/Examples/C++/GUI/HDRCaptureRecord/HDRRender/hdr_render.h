#ifndef _HDR_RENDER_H_
#define _HDR_RENDER_H_

#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class CHdrRender
{
public:
  CHdrRender();
  ~CHdrRender();

public:
  int Open(GLFWwindow* window);
  void Close();
  int Render(GLFWwindow* window, uint32_t width, uint32_t height, uint8_t *data, uint32_t hdr_on);

private:
  void CleanUp();
  void UpdateViewPort(GLFWwindow* window);

  GLint  glsl_v, glsl_f;
  GLint  glsl_v1, glsl_f1;
  GLuint glsl_Program;
  GLuint glsl_Program1;
  GLuint glsl_verLocation;
  GLuint glsl_texLocation;
  GLuint glsl_verLocation1;
  GLuint glsl_texLocation1;
  GLuint glsl_txY, glsl_txUV;
};

#endif//!_HDR_RENDER_H_
