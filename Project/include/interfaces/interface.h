#ifndef INTERFACE_H
#define INTERFACE_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glm/ext.hpp"

#include "shader.h"
#include "texture.h"

// Structure to hold character information
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class Interface {
    public:
        Interface(int width=1280, int height=960);
        ~Interface();
        void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
        void renderImage(Texture* texture, float x, float y, float width, float height, glm::vec4 color = glm::vec4(1.0f));

        int windowWidth;
        int windowHeight;
        Shader* textShader;
        Shader* imageShader;

        FT_Library ft;
        FT_Face face;
        std::map<char, Character> Characters;
        unsigned int VAO, VBO;
        unsigned int imageVAO, imageVBO;
};

#endif