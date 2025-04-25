#include "viewer.h"
#include <filesystem>
#include <iostream>

#include "glm/ext.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ostream>

Viewer::Viewer(int width, int height) {
  if (!glfwInit()) // initialize window system glfw
  {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    glfwTerminate();
  }

  // version hints: create GL window with >= OpenGL 3.3 and core profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  win = glfwCreateWindow(width, height, "Viewer", NULL, NULL);

  if (win == NULL) {
    std::cerr << "Failed to create window" << std::endl;
    glfwTerminate();
  }

  // make win's OpenGL context current; no OpenGL calls can happen before
  glfwMakeContextCurrent(win);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    glfwTerminate();
  }

  // Set user pointer for GLFW window to this Viewer instance
  glfwSetWindowUserPointer(win, this);

  // register event handlers
  glfwSetKeyCallback(win, key_callback_static);
  glfwSetCursorPosCallback(win, mouse_callback_static); // mouse inputs
  glfwSetInputMode(win, GLFW_CURSOR,
                   GLFW_CURSOR_DISABLED); // Hide cursor and lock it

  // useful message to check OpenGL renderer characteristics
  std::cout << glGetString(GL_VERSION) << ", GLSL "
            << glGetString(GL_SHADING_LANGUAGE_VERSION) << ", Renderer "
            << glGetString(GL_RENDERER) << std::endl;

  // initialize GL by setting viewport and default render characteristics
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

  /* tell GL to only draw onto a pixel if the shape is closer to the viewer
  than anything already drawn at that pixel */
  glEnable(GL_DEPTH_TEST); /* enable depth-testing */
  /* with LESS depth-testing interprets a smaller depth value as meaning
   * "closer" */
  glDepthFunc(GL_LESS);

  // Initialize the game
  game = new Game();

  // std::string shader_dir = "/Users/noah-r/Coding/TP4/TP4_material/shaders/";
  /*textShader = Shader(shader_dir + "text.vs", shader_dir+"text.fs"); // path
  relative to binary or CMAKE source dir glm::mat4 projection = glm::ortho(0.0f,
  static_cast<float>(width), 0.0f, static_cast<float>(height));
  textShader.use();
  textShader.setMat4("projection", projection);
  textShader.setInt("text", 0); // Texture unit 0
  // Initialize FreeType
  initFreeType();

  // Set up VAO and VBO for text rendering
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);*/
}

/*void Viewer::initFreeType()
{
    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" <<
std::endl;

    std::string font_path = "/Users/noah-r/Downloads/Roboto/roboto_medium.ttf";
    //if (!std::filesystem::exists(font_path)) {
    //    std::cerr << "ERROR: Font file not found at " << font_path <<
std::endl;
    //    return;
    //}

    if (FT_New_Face(ft, "/Users/noah-r/Downloads/Roboto/roboto_medium.ttf", 0,
&face)) std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;

    if (FT_Set_Pixel_Sizes(face, 0, 48)){
        std::cerr << "ERROR::FREETYPE: Failed to set pixel size" << std::endl;
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Viewer::renderText(std::string text, float x, float y, float scale,
glm::vec3 color)
{
    // Activate corresponding render state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textShader.use();
    textShader.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of
1/64 pixels) x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in
pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}*/

void Viewer::run() {
  const float targetFrameTime = 1.0f / 60.0f; // Target frame time for 60 FPS
  float animationTime = 0.0f;
  // Main render loop for this OpenGL window
  while (!glfwWindowShouldClose(win)) {
    auto frameStart =
        std::chrono::high_resolution_clock::now(); // Stores the start of the
                                                   // frame

    animationTime += 0.08;
    // clear draw buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.keyboard_events(keyStates);
    game->keyHandler(keyStates, glfwGetTime());
    game->updateGame();

    if (!camera.devMode) {
      // camera->cameraFront = ;
      camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
      camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
      camera.cameraPos =
          game->player->position + glm::vec3(0.0f, 0.05f, -0.22f);
      // camera.cameraPos = game->player->position + glm::vec3(0.0f, 0.1f,
      // -0.52f);
    }

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view =
        glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront,
                    camera.cameraUp);
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    game->scene_root->draw(model, view, projection);

    // Render UI
    // renderText("Score: 100", 25.0f, 550.0f, 1.0f,
    // glm::vec3(1.0f, 1.0f, 1.0f));

    // Poll for and process events
    glfwPollEvents();

    // flush render commands, and swap draw buffers
    glfwSwapBuffers(win);

    // Calculate frame time and sleep if necessary
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> frameDuration = frameEnd - frameStart;
    float frameTime = frameDuration.count();

    if (frameTime < targetFrameTime) {
      std::this_thread::sleep_for(std::chrono::milliseconds(
          static_cast<int>((targetFrameTime - frameTime) * 1000)));
    }
  }

  /* close GL context and any other GLFW resources */
  glfwTerminate();
}

void Viewer::key_callback_static(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  Viewer *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
  viewer->on_key(key, action);
}

void Viewer::mouse_callback_static(GLFWwindow *window, double xpos,
                                   double ypos) {
  Viewer *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
  viewer->camera.mouse_callback(xpos, ypos);
}

void Viewer::on_key(int key, int action) {
  // 'Q' or 'Escape' quits
  if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
    glfwSetWindowShouldClose(win, GLFW_TRUE);
  }
  // Store key state (true when pressed, false when released)
  if (action == GLFW_PRESS) {
    if (!keyStates[key].first) {
      keyStates[key].second =
          glfwGetTime(); // store the time when the key started to be pressed
      keyStates[key].first = true;
    }
  } else if (action == GLFW_RELEASE) {
    keyStates[key].first = false;
  }
}
