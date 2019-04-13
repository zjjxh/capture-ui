#include "hdr_render.h"

static char *vShaderStr =
"#version 130 \n"
"attribute vec2 vertexIn; \n"
"attribute vec2 textureIn; \n"
"varying vec2 textureOut; \n"
"void main(void) \n"
"{ \n"
"  gl_Position = vec4(vertexIn, 0.0, 1); \n"
"  textureOut = textureIn; \n"
"}\n";

static char *fShaderStr =
"#version 130 \n"
"varying vec2 textureOut; \n"
"uniform sampler2D tex_y; \n"
"uniform sampler2D tex_uv; \n"
"float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30; \n"
"float hable(float x) { \n"
" return (x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)  - E/F; \n"
"} \n"
"void main(void) \n"
"{ \n"
"  float r, g, b, y, u, v; \n"
"  float max_rgb=0.0; \n"
"  y = texture2D(tex_y, textureOut).r; \n"
"  u = texture2D(tex_uv, textureOut).r; \n"
"  v = texture2D(tex_uv, textureOut).g; \n"
"  \n"
"  r = (y + 1.5396*v - 0.8448); \n"
"  g = (y - 0.1831*u - 0.4577*v + 0.2454); \n"
"  b = (y + 1.8142*u - 0.9821); \n"
"  r = 40.0*pow(r,2.4); \n"
"  g = 40.0*pow(g,2.4); \n"
"  b = 40.0*pow(b,2.4); \n"
"  if(r>max_rgb) max_rgb = r; \n"
"  if(g>max_rgb) max_rgb = g; \n"
"  if(b>max_rgb) max_rgb = b; \n"
"  float ratio = hable(max_rgb) / hable(40.0); \n"
"  ratio = hable(r) / hable(40.0); \n"
"  r = r * ratio / max_rgb; \n"
"  ratio = hable(g) / hable(40.0); \n"
"  g = g * ratio / max_rgb; \n"
"  ratio = hable(b) / hable(40.0); \n"
"  b = b * ratio / max_rgb; \n"
"  \n"
"  gl_FragColor = vec4(r, g, b, 1.0); \n"
"} \n";

static char *v1ShaderStr =
"#version 130 \n"
"attribute vec2 vertexIn; \n"
"attribute vec2 textureIn; \n"
"varying vec2 textureOut; \n"
"void main(void) \n"
"{ \n"
"  gl_Position = vec4(vertexIn, 0.0, 1); \n"
"  textureOut = textureIn; \n"
"}\n";

static char *f1ShaderStr =
"#version  130 \n"
"varying vec2 textureOut; \n"
"uniform sampler2D tex_y; \n"
"uniform sampler2D tex_uv; \n"
"void main(void) \n"
"{ \n"
"  float r, g, b, y, u, v; \n"
"  float max_rgb=0.0; \n"
"  y = texture2D(tex_y, textureOut).r; \n"
"  u = texture2D(tex_uv, textureOut).r; \n"
"  v = texture2D(tex_uv, textureOut).g; \n"
"  \n"
"  y = 1.1643*(y-0.0625); \n"
"  u = u - 0.5; \n"
"  v = v - 0.5; \n"
"  r = y+1.5958*v; \n"
"  g = y-0.39173*u-0.81290*v; \n"
"  b = y+2.017*u; \n"
"  \n"
"  gl_FragColor = vec4(r, g, b, 1.0); \n"
"} \n";

static const GLfloat vertexVertices[] = {
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f,  1.0f,
	1.0f,  1.0f,
};

static const GLfloat textureVertices[] = {
	0.0f,  1.0f,
	1.0f,  1.0f,
	0.0f,  0.0f,
	1.0f,  0.0f,
};


CHdrRender::CHdrRender()
{
  glsl_v = 0;
  glsl_f = 0;
  glsl_v1 = 0;
  glsl_f1 = 0;
  glsl_Program = 0;
  glsl_Program1 = 0;
  glsl_verLocation = 0;
  glsl_texLocation = 0;
  glsl_verLocation1 = 0;
  glsl_texLocation1 = 0;
  glsl_txY = 0;
  glsl_txUV = 0;
}

CHdrRender::~CHdrRender()
{
  CleanUp();
}



void CHdrRender::Close()
{
  CleanUp();
}


void CHdrRender::CleanUp()
{
  if (glsl_v) 
  {
    glDeleteShader(glsl_v);
    glsl_v = 0;
  }

  if (glsl_f) 
  {
    glDeleteShader(glsl_f);
    glsl_f = 0;
  }

  if (glsl_v1) 
  {
    glDeleteShader(glsl_v1);
    glsl_v1 = 0;
  }

  if (glsl_f1) 
  {
    glDeleteShader(glsl_f1);
    glsl_f1 = 0;
  }

  if (glsl_Program) 
  {
    glDeleteProgram(glsl_Program);
    glsl_Program = 0;
  }

  if (glsl_Program1) 
  {
    glDeleteProgram(glsl_Program1);
    glsl_Program1 = 0;
  }

  if (glsl_txY) 
  {
    glDeleteTextures(1, &glsl_txY);
    glsl_txY = 0;
  }

  if (glsl_txUV) 
  {
    glDeleteTextures(1, &glsl_txUV);
    glsl_txUV = 0;
  }
}

int CHdrRender::Open(GLFWwindow* window)
{
/*	
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  GLenum glew_err = glewInit();
  if(GLEW_OK != glew_err)
  {
    return 1;
  }
*/
  GLint vertCompiled = 0, fragCompiled = 0, linked = 0;

  glGenTextures(1, &glsl_txY);
  if (!glsl_txY)
  {
    CleanUp();
    return 2;
  }
  glBindTexture(GL_TEXTURE_2D, glsl_txY);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &glsl_txUV);
  if (!glsl_txUV)
  {
    CleanUp();
    return 2;
  }
  glBindTexture(GL_TEXTURE_2D, glsl_txUV);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glsl_v = glCreateShader(GL_VERTEX_SHADER);
  if (!glsl_v)
  {
    CleanUp();
    return 3;
  }

  glsl_f = glCreateShader(GL_FRAGMENT_SHADER);
  if (!glsl_f)
  {
    CleanUp();
    return 3;   
  }

  glShaderSource(glsl_v, 1, (const GLchar **)&vShaderStr, NULL);
  glShaderSource(glsl_f, 1, (const GLchar **)&fShaderStr, NULL);

  glCompileShader(glsl_v);
  glGetShaderiv(glsl_v, GL_COMPILE_STATUS, &vertCompiled);
  if (!vertCompiled)
  {
    CleanUp();
    return 4;
  }

  glCompileShader(glsl_f);
  glGetShaderiv(glsl_f, GL_COMPILE_STATUS, &fragCompiled);
  if (!fragCompiled)
  {
    CleanUp();
    return 4;
  }

  glsl_Program = glCreateProgram();
  glAttachShader(glsl_Program, glsl_v);
  glAttachShader(glsl_Program, glsl_f);

  glLinkProgram(glsl_Program);
  glGetProgramiv(glsl_Program, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    CleanUp();
    return 5;
  }

  glsl_verLocation = glGetAttribLocation(glsl_Program, "vertexIn");
  glsl_texLocation = glGetAttribLocation(glsl_Program, "textureIn");
  glVertexAttribPointer(glsl_verLocation, 2, GL_FLOAT, 0, 0, vertexVertices);
  glEnableVertexAttribArray(glsl_verLocation);
  glVertexAttribPointer(glsl_texLocation, 2, GL_FLOAT, 0, 0, textureVertices);
  glEnableVertexAttribArray(glsl_texLocation);


  glsl_v1 = glCreateShader(GL_VERTEX_SHADER);
  if (!glsl_v1)
  {
    CleanUp();
    return 3;
  }

  glsl_f1 = glCreateShader(GL_FRAGMENT_SHADER);
  if (!glsl_f1)
  {
    CleanUp();
    return 3;   
  }

  glShaderSource(glsl_v1, 1, (const GLchar **)&v1ShaderStr, NULL);
  glShaderSource(glsl_f1, 1, (const GLchar **)&f1ShaderStr, NULL);

  glCompileShader(glsl_v1);
  glGetShaderiv(glsl_v1, GL_COMPILE_STATUS, &vertCompiled);
  if (!vertCompiled)
  {
    CleanUp();
    return 4;
  }

  glCompileShader(glsl_f1);
  glGetShaderiv(glsl_f1, GL_COMPILE_STATUS, &fragCompiled);
  if (!fragCompiled)
  {
    CleanUp();
    return 4;
  }

  glsl_Program1 = glCreateProgram();
  glAttachShader(glsl_Program1, glsl_v1);
  glAttachShader(glsl_Program1, glsl_f1);

  glLinkProgram(glsl_Program1);
  glGetProgramiv(glsl_Program1, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    CleanUp();
    return 5;
  }

  glsl_verLocation1 = glGetAttribLocation(glsl_Program1, "vertexIn");
  glsl_texLocation1 = glGetAttribLocation(glsl_Program1, "textureIn");
  glVertexAttribPointer(glsl_verLocation1, 2, GL_FLOAT, 0, 0, vertexVertices);
  glEnableVertexAttribArray(glsl_verLocation1);
  glVertexAttribPointer(glsl_texLocation1, 2, GL_FLOAT, 0, 0, textureVertices);
  glEnableVertexAttribArray(glsl_texLocation1);

  glClearColor(0.2f,0.3f,0.3f,1.0f);
  return 0;
}

void CHdrRender::UpdateViewPort(GLFWwindow* window)
{
  int Wwidth,Wheight;
  glfwGetFramebufferSize(window,&Wwidth,&Wheight);
  glViewport(0,0,Wwidth,Wheight);
}

int CHdrRender::Render(GLFWwindow* window, uint32_t width, uint32_t height, uint8_t *data, uint32_t hdr_on)
{
  GLenum err = GL_NO_ERROR;
  //glfwMakeContextCurrent(window);
  /*UpdateViewPort(window);
  
  glClear(GL_COLOR_BUFFER_BIT);    
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 1;
  }
*/
  if(hdr_on)
    glUseProgram(glsl_Program);
  else
    glUseProgram(glsl_Program1);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, glsl_txY);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 2;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height,
               0, GL_RED, GL_UNSIGNED_SHORT, data);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 3;
  }
  glUniform1i(glGetUniformLocation(glsl_Program, "tex_y"), 0);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 4;
  }

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, glsl_txUV);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 2;
  }
 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, (width>>1), (height>>1),
               0, GL_RG, GL_UNSIGNED_SHORT, data+(2*width*height));
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 3;
  }
  glUniform1i(glGetUniformLocation(glsl_Program, "tex_uv"), 1);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 4;
  }

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 5;
  }
/*
  glfwSwapBuffers(window); 
  err = glGetError();
  if(err!=GL_NO_ERROR)
  {
    CleanUp();
    return 6;
  }
*/
  return 0;
}



