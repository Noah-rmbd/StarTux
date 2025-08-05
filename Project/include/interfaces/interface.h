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
#include <glm/gtc/matrix_transform.hpp>
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

// Enum for different rendering layers
enum class RenderLayer {
    BACKGROUND = 0,
    HUD_ELEMENTS,
    TEXT_OVERLAY,
    CURSOR
};

// Structure for HUD panel configuration
struct HudPanel {
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles in degrees
    glm::vec2 size;
    Texture* backgroundTexture;
    bool is3D;
};

// Structure for text overlay element
struct TextOverlay {
    std::string text;
    glm::vec3 position;  // 3D position for 3D HUD, 2D for screen space
    float scale;
    glm::vec3 color;
    bool is3D;  // If true, renders in 3D space, otherwise screen space
    int panelId;  // Which panel this text belongs to (-1 for screen space)
};

// Structure for image element
struct ImageElement {
    Texture* texture;
    glm::vec3 position;
    glm::vec2 size;
    glm::vec4 color;
    bool is3D;
    int panelId;
};

class Interface {
    public:
        Interface(int width=1480, int height=960);
        ~Interface();
        
        // Legacy 2D rendering methods (for backward compatibility)
        void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
        void renderImage(Texture* texture, float x, float y, float width, float height, glm::vec4 color = glm::vec4(1.0f));
        
        // New layered rendering system
        void beginFrame(glm::mat4 view, glm::mat4 projection);
        void endFrame();
        
        // Panel management
        int createPanel(glm::vec3 position, glm::vec3 rotation, glm::vec2 size, Texture* background = nullptr);
        void updatePanel(int panelId, glm::vec3 position, glm::vec3 rotation);
        
        // Element management
        void addTextOverlay(const std::string& text, glm::vec3 position, float scale, glm::vec3 color, bool is3D = false, int panelId = -1);
        void addImageElement(Texture* texture, glm::vec3 position, glm::vec2 size, glm::vec4 color = glm::vec4(1.0f), bool is3D = false, int panelId = -1);
        void addCursorElement(Texture* texture, glm::vec3 position, glm::vec2 size, glm::vec4 color = glm::vec4(1.0f));
        
        // Clear methods
        void clearTextOverlays();
        void clearImageElements();
        void clearCursorElements();
        void clearPanel(int panelId);
        
        // Rendering methods
        void renderLayer(RenderLayer layer);
        
        int windowWidth;
        int windowHeight;
        Shader* textShader;
        Shader* imageShader;
        Shader* text3DShader;
        Shader* image3DShader;

        FT_Library ft;
        FT_Face face;
        std::map<char, Character> Characters;
        unsigned int VAO, VBO;
        unsigned int imageVAO, imageVBO;
        
    private:
        // Current frame matrices
        glm::mat4 currentView;
        glm::mat4 currentProjection;
        
        // Storage for elements
        std::vector<HudPanel> panels;
        std::vector<TextOverlay> textOverlays;
        std::vector<ImageElement> imageElements;
        std::vector<ImageElement> cursorElements;  // Separate storage for cursor elements
        
        // Internal rendering methods
        void renderText3D(const TextOverlay& textOverlay);
        void renderImage3D(const ImageElement& imageElement);
        void renderPanel(const HudPanel& panel);
        
        // Helper methods
        glm::mat4 getPanelMatrix(const HudPanel& panel);
        void setupBlending();
        void disableBlending();
};

#endif