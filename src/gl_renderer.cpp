#include "render_interface.h"
#ifdef WEB_BUILD
#include "GLES3/gl3.h"
#else

// This shit adds +1s compilation time
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #############################################################################
//                           OpenGL constants
// #############################################################################
const char* TEXTURE_PATH = "assets/textures/atlas.png";
constexpr Vec2 LIGHT_MAP_SIZE = {142, 72};

// #############################################################################
//                           OpenGL structs
// #############################################################################
struct Texture
{
  GLuint ID;
  GLenum binding;
  int width;
  int height;
  long long fileTS;
};

struct Program
{
  GLuint ID;
  char* vertShader;
  char* fragShader;
  long long realoadTS;
};

struct Framebuffer
{
  GLuint ID;
  int attachmentCount;
  Texture* colorAttachments[10];
  Texture* depthAttachment = nullptr;
};

struct OpenGLContext
{
  // Textures
  Texture textureAtlas;
  Program mainProgram;  

  // Vertext Buffer Object
  GLuint VAO = 0;
  GLuint VBO = 0;

  GLuint globalUBO;

  Texture lightMap;
  Framebuffer lightMapFB;
  Program lightProgram;

  // Font
  Program fontProgram;
  Framebuffer uiFB;
  Program finalProgram;


  Texture fontArray;
  Texture fontTextures[FONT_COUNT];
  Texture depthTexture;
  Texture uiTexture;
};

// #############################################################################
//                           OpenGL Globals
// #############################################################################
static OpenGLContext glContext = {};

// #############################################################################
//                           OpenGL Functions
// #############################################################################
Texture gl_create_texture(GLint staticFormat, GLenum binding, 
                          int width, int height, char* pixels = NULL,
                          GLenum format = GL_RGBA,
                          GLenum type = GL_UNSIGNED_BYTE, 
                          // GLenum wrapping = GL_CLAMP_TO_BORDER,
                          GLenum wrapping = GL_CLAMP_TO_EDGE,
                          GLenum filtering = GL_LINEAR)
{
  // SM_TRACE("Creating OpenGL Texture Format: %d, binding: %d, w: %d, h: %d, format: %d, type: %d, wrapping: %d, filtering: %d",
  //          staticFormat, binding, width, height, format, type, wrapping, filtering);
  Texture texture = {.binding = binding, .width = width, .height = height};
  glGenTextures(1, &texture.ID);
  // SM_ASSERT(texture.ID, "Failed to create texture");

  glActiveTexture(binding);
  glBindTexture(GL_TEXTURE_2D, texture.ID);

  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
  // This setting only matters when using the GLSL texture() function
  // When you use texelFetch() this setting has no effect,
  // because texelFetch is designed for this purpose
  // See: https://interactiveimmersive.io/blog/glsl/glsl-data-tricks/
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

  glTexImage2D(GL_TEXTURE_2D, 0, staticFormat, width, height, 
                0, format, type, pixels);

  return texture;
}

GLuint gl_create_shader(int shaderType, char* shaderPath)
{
  SDL_Log("Creating Shader: %s, Type: %d", shaderPath, shaderType);


  // char* shaderHeader = read_file("src/shader_header.h", &fileSize, transientStorage);
  // if(!shaderHeader)
  // {
  //   SM_ASSERT(false, "Failed to load shader header: src/shaders_header.h");
  //   return 0;
  // }

  // char* shader = read_file(shaderPath, &fileSize, transientStorage);
  size_t fileSize = 0;
  char* shader = (char*)SDL_LoadFile(shaderPath, &fileSize);
  if(!shader)
  {
    SDL_Log("Failed to load shader: %s",shaderPath);
    return 0;
  }

  char* shaderSources[] =
  {
    // "#version 430 core\n",
    // shaderHeader,
    // "\n#line 1\n",
    shader
  };

  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID, ArraySize(shaderSources), shaderSources, 0);
  glCompileShader(shaderID);

  // Test if Shader compiled successfully 
  {
    int success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      char shaderLog[2048] = {};
      glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);
      SDL_Log("Failed to compile %s Shader, Error: %s", shaderPath, shaderLog);
      return 0;
    }
  }

  return shaderID;
}

// Program gl_create_program(char *vertShaderPath, char *fragShaderPath, BumpAllocator *transientStorage)
Program gl_create_program(char *vertShaderPath, char *fragShaderPath)
{
  // SM_TRACE("Creating Program. Vert: %s", vertShaderPath);
  // SM_TRACE("Creating Program. Frag: %s", fragShaderPath);

  Program program = {
    .vertShader = vertShaderPath,
    .fragShader = fragShaderPath,
    .realoadTS = get_timestamp(vertShaderPath)
  };

  // Program ID
  {
    GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, vertShaderPath);
    GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, fragShaderPath); 

    if(!vertShaderID || !fragShaderID)
    {
      SDL_Log("Failed to create Shaders");
      return {};
    }

    GLuint programID = glCreateProgram();
    if(!programID)
    {
      SDL_Log("Failed to call glCreateProgram");
      return program;
    }
    glAttachShader(programID, vertShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);

    glDetachShader(programID, vertShaderID);
    glDetachShader(programID, fragShaderID);
    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    // int status;
    // glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);
    // if(!status)
    // {
    //   char programInfoLog[512];
    //   glGetProgramInfoLog(programID, 512, 0, programInfoLog);

    //   SDL_Log("Failed to link program: %s", programInfoLog);
    //   glDeleteProgram(programID);
    //   return {};
    // }

    // Validate if program works
    // #ifdef DEBUG
    {
      int programSuccess;
      char programInfoLog[512];
      glGetProgramiv(programID, GL_LINK_STATUS, &programSuccess);

      if(!programSuccess)
      {
        glGetProgramInfoLog(programID, 512, 0, programInfoLog);

        SDL_Log("Failed to link program: %s", programInfoLog);
        glDeleteProgram(programID);
        return {};
      }
    }
    // #endif

    program.ID = programID;
  }

  return program;
}

// Create UI framebuffer
Framebuffer gl_create_framebuffer(Texture* colorAttachments, int count, 
                                  Texture* depthAttachment = nullptr, bool srgb = true)
{
  Framebuffer framebuffer = {.depthAttachment = depthAttachment};

  if(!colorAttachments || count <= 0)
  {
    SDL_Log("No color attachments supplied to Framebuffer!");
    return {}; // Zero
  }

  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  glGenFramebuffers(1, &framebuffer.ID);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.ID);

  // Set the list of draw buffers.
  const GLenum drawBuffers[] = 
  {
    GL_COLOR_ATTACHMENT0, 
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,
    GL_COLOR_ATTACHMENT5,
    GL_COLOR_ATTACHMENT6,
    GL_COLOR_ATTACHMENT7,
    GL_COLOR_ATTACHMENT8,
    GL_COLOR_ATTACHMENT9,
  };

  // Set "game" Texture as our colour attachement #0
  for(int attachmentIdx = 0; attachmentIdx < count; attachmentIdx++)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[attachmentIdx], GL_TEXTURE_2D, 
                           colorAttachments[attachmentIdx].ID, 0);
  }

  // Set "depth" Texture as our depth attachment 
  if(depthAttachment)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                            GL_TEXTURE_2D, depthAttachment->ID, 0);
  }
  glDrawBuffers(count, drawBuffers);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    SDL_Log("Framebuffer incomplete!");
    return {}; // Zero;
  }

  // sRGB output (even if input texture is non-sRGB -> don't rely on texture used)
  // Your font is not using sRGB, for example (not that it matters there, because no actual color is sampled from it)
  // But this could prevent some future bug when you start mixing different types of textures
  // Of course, you still need to correctly set the image file source format when using glTexImage2D()
  // glEnable(GL_FRAMEBUFFER_SRGB);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return framebuffer;
}

#ifndef WEB_BUILD
void glad_callback(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...)
{
  GLenum error = glad_glGetError(); 
  if(error != GL_NO_ERROR)
  {
    SDL_Log("OpenGL Error detected %d in function name: %s", error, name);
  }
}
#endif

// #############################################################################
//                                Init
// #############################################################################
bool gl_init(SDL_Window* window)
{
	#ifndef WEB_BUILD
	gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
	#endif

  SDL_Log("OpenGL -> Version: %s", glGetString(GL_VERSION));
  SDL_Log("GPU -> Vendor: %s, Model: %s",  glGetString(GL_VENDOR), glGetString(GL_RENDERER));
  SDL_Log("Shading language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Initialize debugging
  #ifndef WEB_BUILD
  {
    SDL_Log("Setting up OpenGL Debugger...");
    gladSetGLPostCallback(&glad_callback);
  }
  #endif

  // Load Texture Atlas
  {
    // SM_TRACE("Loading Texture Atlas...");
    int width, height, nChannels;
    char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &nChannels, 4);

    if(!data)
    {
      // SM_ASSERT(0, "Failed to load Texture!");
      return false;
    }

    // Generate OpenGL Texture
    glContext.textureAtlas = 
      gl_create_texture(GL_RGBA, GL_TEXTURE0, width, height, data, GL_RGBA, GL_UNSIGNED_BYTE,
                        GL_CLAMP_TO_EDGE, GL_LINEAR);
    glContext.textureAtlas.fileTS = get_timestamp(TEXTURE_PATH);
    stbi_image_free(data);
  }

  // Textures
  {
    glContext.lightMap =
      gl_create_texture(GL_RGBA8,
                        GL_TEXTURE1,
                        LIGHT_MAP_SIZE.x, LIGHT_MAP_SIZE.y, 0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        GL_CLAMP_TO_EDGE,
                        GL_LINEAR);

    glContext.uiTexture =
      gl_create_texture(GL_RGBA8,
                        GL_TEXTURE3,
                        renderData.uiSpace.x, renderData.uiSpace.y, 0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        GL_CLAMP_TO_EDGE,
                        // GL_NEAREST);
                        GL_LINEAR);


    // Depth Texture
    glContext.depthTexture = 
      gl_create_texture(GL_DEPTH_COMPONENT24, 
                        GL_TEXTURE20,
                        renderData.globalData.windowSize.x, renderData.globalData.windowSize.y, 0,
                        GL_DEPTH_COMPONENT,
                        GL_UNSIGNED_INT,
                        GL_CLAMP_TO_EDGE,
                        GL_NEAREST);
    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // Init Font Array
  {
    SDL_Log("Creating OpenGL Texture Array");
    glGenTextures(1, &glContext.fontArray.ID);
    ASSERT_MSG(glContext.fontArray.ID, "Failed to create texture array for Fonts");
    glBindTexture(GL_TEXTURE_2D_ARRAY, glContext.fontArray.ID);

    // Allocate the memory
	  glTexImage3D(GL_TEXTURE_2D_ARRAY,
		0,                                              // level
		GL_RG8,                                         // Internal format
		FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, FONT_COUNT,   // width,height,depth
		0,                
		GL_RG,                                          // format
		GL_UNSIGNED_BYTE,                               // type
		0);                                             // pointer to data
  }

  // Framebuffers
  {
    // Light Map
    Texture lightMapAttachments[] = {glContext.lightMap};
    glContext.lightMapFB = gl_create_framebuffer(lightMapAttachments, ArraySize(lightMapAttachments));
    
    // UI
    Texture uiAttachments[] = {glContext.uiTexture};
    glContext.uiFB = gl_create_framebuffer(uiAttachments, ArraySize(uiAttachments));
  }

  // Create program
  glContext.mainProgram = gl_create_program("assets/shaders/quad.vert", "assets/shaders/quad.frag");
  glContext.lightProgram = gl_create_program("assets/shaders/fullscreen.vert", "assets/shaders/lightmap.frag");
  glContext.fontProgram = gl_create_program("assets/shaders/quad.vert", "assets/shaders/font.frag");
  glContext.finalProgram = gl_create_program("assets/shaders/fullscreen.vert", "assets/shaders/final.frag");

  // Set Uniform locations
  {
    // Set sampler in shader to texture unit 1 (the unit that texture was created with)
    // FOR THIS FKING PROGRAM ONLY BTW!!!!!!!!!!!!!!!!!!
    glUseProgram(glContext.lightProgram.ID);
    GLint uniformLocation = glGetUniformLocation(glContext.lightProgram.ID, "lightMap");
    glUniform1i(uniformLocation, 1);

    glUseProgram(glContext.fontProgram.ID);
    uniformLocation = glGetUniformLocation(glContext.fontProgram.ID, "fontArray");
    glUniform1i(uniformLocation, 2);

    glUseProgram(glContext.finalProgram.ID);
    uniformLocation = glGetUniformLocation(glContext.finalProgram.ID, "uiTexture");
    glUniform1i(uniformLocation, 3);
  }

  // Generate Vertex Buffer Object
  // VBO -> Buffer (Data)
  glGenBuffers(1, &glContext.VBO);
  glBindBuffer(GL_ARRAY_BUFFER, glContext.VBO);

  // Do layout stuff
  // VAO -> Layout
  {
    glGenVertexArrays(1, &glContext.VAO);
    glBindVertexArray(glContext.VAO);
    int attribIdx = 0;
    u8* offset = nullptr;

    // Atlas Position
    glVertexAttribIPointer(attribIdx, 1, GL_INT, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    // glVertexAttribPointer(attribIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Transform), offset);
    // glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(int) * 1;

    // Sprite Size
    // glVertexAttribIPointer(attribIdx, 2, GL_INT, sizeof(Transform), offset);
    // glEnableVertexAttribArray(attribIdx);
    glVertexAttribPointer(attribIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(int) * 2;

    // Position
    glVertexAttribPointer(attribIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(float) * 2;

    // Size
    glVertexAttribPointer(attribIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(float) * 2;

    // MatrixIdx
    glVertexAttribIPointer(attribIdx, 1, GL_INT, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(int);

    // FontIdx
    glVertexAttribIPointer(attribIdx, 1, GL_INT, sizeof(Transform), offset);
    glEnableVertexAttribArray(attribIdx);
    glVertexAttribDivisor(attribIdx, 1); // the 1 means this data is per instance rather than per vertex
    attribIdx += 1;
    offset += sizeof(int);

    glBindVertexArray(0); // Unbind
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(Transform) * MAX_TRANSFORMS, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind

  // Global Data
  {
    glGenBuffers(1, &glContext.globalUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, glContext.globalUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalData), &renderData.globalData, GL_DYNAMIC_DRAW);
  }

  // sRGB output (even if input texture is non-sRGB -> don't rely on texture used)
  // Your font is not using sRGB, for example (not that it matters there, because no actual color is sampled from it)
  // But this could prevent some future bug when you start mixing different types of textures
  // Of course, you still need to correctly set the image file source format when using glTexImage2D()
  // SM_TRACE("Enabling GL_FRAMEBUFFER_SRGB");
  // glEnable(GL_FRAMEBUFFER_SRGB);
  // glDisable(GL_MULTISAMPLE); // disable multisampling

  load_font(FONT_HEADING, "assets/fonts/alagard.ttf", 16, 1);
  load_font(FONT_TEXT, "assets/fonts/NameHereCondensed.ttf", 16, 1);


  return true;
};

// #############################################################################
//                                Render
// #############################################################################
void gl_render(SDL_Window* window)
{
  // Reload Fonts if needed
  for(int i = 0; i < FONT_COUNT; i++)
  {
    Font& font = renderData.fonts[i];
    if(font.reload)
    {
      Texture& texture = glContext.fontTextures[i];
      if(!texture.ID)
      {
        // Upload new data

      }
      // Bind font array
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D_ARRAY, glContext.fontArray.ID);

      // Upload data
      // It's simple math. Your data must be width * height * depth (in channel Idx: zoffset)
      const int slice = i;
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0/*level*/, 0/*xoffset*/, 0/*yOffset*/, slice/*zoffset*/,
                      FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 1, 
                      GL_RG, GL_UNSIGNED_BYTE, font.bitMap);

      // Always set reasonable texture parameters
      glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

      font.reload = false;
    }
  }

  // Calc Lightmap Camera & upload GlobalData to GPU
  {
    renderData.lightsCam.dimensions = LIGHT_MAP_SIZE;
    renderData.lightsCam.position = renderData.gameCam.position;
    renderData.lightsCam.zoom = max(1.0f, renderData.gameCam.zoom - 1.0f);

    const OrthographicCamera2D& lightsCam = renderData.lightsCam;
    float zoom = lightsCam.zoom? lightsCam.zoom : 1.0f;
    Vec2 dimensions =  lightsCam.dimensions / zoom;
    Vec2 pos = lightsCam.position;

    renderData.globalData.orthProjGame[ORTHO_PROJ_LIGHTS] =
      orthographic_projection(pos.x-dimensions.x / 2.0f,
                              pos.x+dimensions.x / 2.0f,
                              -pos.y-dimensions.y / 2.0f,
                              -pos.y+dimensions.y / 2.0f);
  }

  // Resize UI Texture
  if(glContext.uiTexture.width != renderData.uiSpace.x || 
     glContext.uiTexture.height != renderData.uiSpace.y)
  {
    glBindTexture(GL_TEXTURE_2D, glContext.uiTexture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderData.uiSpace.x, renderData.uiSpace.y, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // glContext.uiTexture =
    //   gl_create_texture(GL_RGBA8,
    //                     GL_TEXTURE3,
    //                     renderData.globalData.windowSize.x, renderData.globalData.windowSize.y, 0,
    //                     // 1220, 686, 0,
    //                     GL_RGBA,
    //                     GL_UNSIGNED_BYTE,
    //                     GL_CLAMP_TO_EDGE,
    //                     // GL_NEAREST);
    //                     GL_LINEAR);
  }

  glUseProgram(glContext.mainProgram.ID);
  glBindTexture(GL_TEXTURE_2D, glContext.textureAtlas.ID);

  glBindVertexArray(glContext.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, glContext.VBO);

  // Window changed size, query size again
  {
    SDL_GetWindowSize(window, &renderData.globalData.windowSize.x, &renderData.globalData.windowSize.y);
  }
  glViewport(0, 0, renderData.globalData.windowSize.x, renderData.globalData.windowSize.y);

  // Needs to be enabled so clear depth works, stupid
  glDepthMask(GL_TRUE);
  glClearColor(0, 0, 0, 1);
  // glClearDepthf(0.0f);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT);

  // Copy Global Data to the GPU
  glBindBufferBase(GL_UNIFORM_BUFFER, 4, glContext.globalUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalData), &renderData.globalData);

  // Render Game
  if(renderData.transformCount > 0)
  {
    // Upload transforms (Into Vertext Array)
    const u64 size = sizeof(Transform) * renderData.transformCount;
    const u32 offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, &renderData.transforms[0]);
    
    // Draw Transforms
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData.transformCount);

    // Clear Data
    renderData.transformCount = 0;
  }

  // Apply Lighting
  if(renderData.lightCount > 0)
  {
    glViewport(0, 0, LIGHT_MAP_SIZE.x,  LIGHT_MAP_SIZE.y);
    // glUseProgram(glContext.gameProgram.ID);
    glBindFramebuffer(GL_FRAMEBUFFER, glContext.lightMapFB.ID);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);

    // Clear Color
    {
      const Vec4 ambientColor = {180.0f/255.0f, 180.0f/255.0f, 0.0f, 180.0f/255.0f};
      glClearColor(ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a);
      glClear(GL_COLOR_BUFFER_BIT);
    }

    // Upload transforms (Into Vertext Array)
    const u64 size = sizeof(Transform) * renderData.lightCount;
    const u32 offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, &renderData.lights[0]);
    
    // Draw Transforms
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData.lightCount);

    // Clear Data
    renderData.lightCount = 0;
    glDisable(GL_BLEND);
  }

  // Back buffer again
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // https://www.reddit.com/r/opengl/comments/ejwqa8/opengl_glfw_program_breaks_when_nondefault/
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  // Render Light Map on top
  {
    glUseProgram(glContext.lightProgram.ID);
    glBindTexture(GL_TEXTURE_2D, glContext.lightMap.ID);
    glViewport(0, 0, renderData.globalData.windowSize.x, renderData.globalData.windowSize.y);

    // Blending
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_COLOR, GL_ONE, GL_ONE, GL_ZERO);
    glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
  }

  // Render UI after Lights
  if(renderData.uiTransformCount > 0)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, glContext.uiFB.ID);
    glUseProgram(glContext.fontProgram.ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, glContext.fontArray.ID);
    glViewport(0, 0, renderData.uiSpace.x, renderData.uiSpace.y);
    // glViewport(0, 0, 1220, 686);

    // Clear Color
    {
      // const Vec4 uiClearColor = {253.0f/255.0f, 192.0f/255.0f, 116.0f/255.0f, 255.0f/255.0f};
      const Vec4 uiClearColor = {0, 0, 0, 0};
      glClearColor(uiClearColor.r, uiClearColor.g, uiClearColor.b, uiClearColor.a);
      glClear(GL_COLOR_BUFFER_BIT);
    }

    // Upload transforms (Into Vertext Array)
    const u64 size = sizeof(Transform) * renderData.uiTransformCount;
    const u32 offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, &renderData.uiTransforms[0]);
    
    // Draw Transforms
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData.uiTransformCount);

    // Clear Data
    renderData.uiTransformCount = 0;
  }

  // Final Merge between Game & UI
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(glContext.finalProgram.ID);
  glBindTexture(GL_TEXTURE_2D, glContext.uiTexture.ID);
  glViewport(0, 0, renderData.globalData.windowSize.x, renderData.globalData.windowSize.y);

  glEnable(GL_BLEND);

  //glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
  glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisable(GL_BLEND);

  // FPS limit
  #ifndef WEB_BUILD
  static u64 startTick = SDL_GetPerformanceCounter();
  static u64 ticksPerSecond = SDL_GetPerformanceFrequency();
  static const double secondsPerTick = 1.0 / ticksPerSecond;
  constexpr double limit = 120.0;

  u64 currentTick = SDL_GetPerformanceCounter();
  const double elapsedSeconds = (currentTick - startTick) * secondsPerTick;

  precise_wait_seconds(1.0 / limit - elapsedSeconds);
  startTick = SDL_GetPerformanceCounter();
  #endif
}