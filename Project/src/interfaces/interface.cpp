#include "interface.h"
#include <filesystem>
#include <iostream>
#include <algorithm>
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Interface::Interface(int width, int height) : windowWidth(width), windowHeight(height)
{
    std::string shader_dir = SHADER_DIR;
    std::string textures_dir = TEXTURES_DIR;
    std::string ressources_dir = RESSOURCES_DIR;

    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    std::string font_path = ressources_dir + "Roboto-Medium.ttf";

    if (FT_New_Face(ft, font_path.c_str(), 0, &face))
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;

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

        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    // Initialize 2D shaders (legacy support)
    textShader = new Shader(shader_dir + "text.vs", 
                            shader_dir + "text.fs");
    imageShader = new Shader(shader_dir + "texture.vert", 
                           shader_dir + "texture.frag");
    
    // Initialize 3D shaders for new system
    text3DShader = new Shader(shader_dir + "text.vs", 
                             shader_dir + "text.fs");
    image3DShader = new Shader(shader_dir + "texture.vert", 
                              shader_dir + "texture.frag");

    // Set up 2D text shader projection with Z range for depth testing
    glm::mat4 projection2D = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), -1.0f, 1.0f);
    textShader->use();
    textShader->setMat4("projection", projection2D);
    textShader->setInt("text", 0);

    // Set up 2D image shader projection and view (legacy)
    imageShader->use();
    imageShader->setMat4("projection", projection2D);
    glm::mat4 view2D = glm::mat4(1.0f);
    imageShader->setMat4("view", view2D);
    imageShader->setInt("diffuse_map", 0);
    
    // Set up 3D shaders (will be configured per frame)
    text3DShader->use();
    text3DShader->setInt("text", 0);
    
    image3DShader->use();
    image3DShader->setInt("diffuse_map", 0);

    // Set up VAO and VBO for text rendering
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set up VAO and VBO for image rendering
    glGenVertexArrays(1, &imageVAO);
    glGenBuffers(1, &imageVBO);
    glBindVertexArray(imageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    
    // Simple quad vertices - using position as texture coordinates for the current shader
    // Fix both horizontal and vertical mirroring
    float vertices[] = {
        // positions (which will be used as texture coordinates by the shader)
         0.0f,  0.0f, 0.0f,   // top right (flipped both X and Y)
         0.0f,  1.0f, 0.0f,   // bottom right (flipped both X and Y)
         1.0f,  1.0f, 0.0f,   // bottom left (flipped both X and Y)
         1.0f,  0.0f, 0.0f    // top left (flipped both X and Y)
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute (shader uses this as texture coordinates)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}

void Interface::renderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    // Activate corresponding render state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textShader->use();
    textShader->setVec3("textColor", color);
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
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void Interface::renderImage(Texture* texture, float x, float y, float width, float height, glm::vec4 color)
{
    // Activate shader
    imageShader->use();
    
    // Create model matrix for scaling and translation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));  // Z is still 0 for legacy method
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    
    // Set uniforms
    imageShader->setMat4("model", model);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->getGLid());
    
    // Draw quad
    glBindVertexArray(imageVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Disable blending
    glDisable(GL_BLEND);
}

// New layered rendering methods
void Interface::beginFrame(glm::mat4 view, glm::mat4 projection) {
    currentView = view;
    currentProjection = projection;
    
    // Clear previous frame elements
    textOverlays.clear();
    imageElements.clear();
    cursorElements.clear();
}

void Interface::endFrame() {
    // Disable depth testing for 2D interface rendering to avoid interfering with 3D game objects
    glDisable(GL_DEPTH_TEST);
    
    // Render all layers in order (CURSOR before TEXT_OVERLAY so cursor is behind text)
    renderLayer(RenderLayer::BACKGROUND);
    renderLayer(RenderLayer::HUD_ELEMENTS);
    renderLayer(RenderLayer::CURSOR);
    renderLayer(RenderLayer::TEXT_OVERLAY);
    
    // Re-enable depth testing for the main game rendering
    glEnable(GL_DEPTH_TEST);
}

// Panel management
int Interface::createPanel(glm::vec3 position, glm::vec3 rotation, glm::vec2 size, Texture* background) {
    HudPanel panel;
    panel.position = position;
    panel.rotation = rotation;
    panel.size = size;
    panel.backgroundTexture = background;
    panel.is3D = true;
    
    panels.push_back(panel);
    return panels.size() - 1;
}

void Interface::updatePanel(int panelId, glm::vec3 position, glm::vec3 rotation) {
    if (panelId >= 0 && panelId < panels.size()) {
        panels[panelId].position = position;
        panels[panelId].rotation = rotation;
    }
}

// Element management
void Interface::addTextOverlay(const std::string& text, glm::vec3 position, float scale, glm::vec3 color, bool is3D, int panelId) {
    TextOverlay overlay;
    overlay.text = text;
    overlay.position = position;
    overlay.scale = scale;
    overlay.color = color;
    overlay.is3D = is3D;
    overlay.panelId = panelId;
    
    textOverlays.push_back(overlay);
}

void Interface::addImageElement(Texture* texture, glm::vec3 position, glm::vec2 size, glm::vec4 color, bool is3D, int panelId) {
    ImageElement element;
    element.texture = texture;
    element.position = position;
    element.size = size;
    element.color = color;
    element.is3D = is3D;
    element.panelId = panelId;
    
    imageElements.push_back(element);
}

void Interface::addCursorElement(Texture* texture, glm::vec3 position, glm::vec2 size, glm::vec4 color) {
    ImageElement element;
    element.texture = texture;
    element.position = position;
    element.size = size;
    element.color = color;
    element.is3D = false;  // Cursor is always 2D screen space
    element.panelId = -1;  // No panel attachment
    
    cursorElements.push_back(element);
}

// Clear methods
void Interface::clearTextOverlays() {
    textOverlays.clear();
}

void Interface::clearImageElements() {
    imageElements.clear();
}

void Interface::clearCursorElements() {
    cursorElements.clear();
}

void Interface::clearPanel(int panelId) {
    // Remove all elements belonging to this panel
    textOverlays.erase(
        std::remove_if(textOverlays.begin(), textOverlays.end(),
                      [panelId](const TextOverlay& overlay) { return overlay.panelId == panelId; }),
        textOverlays.end()
    );
    
    imageElements.erase(
        std::remove_if(imageElements.begin(), imageElements.end(),
                      [panelId](const ImageElement& element) { return element.panelId == panelId; }),
        imageElements.end()
    );
}

// Layer rendering
void Interface::renderLayer(RenderLayer layer) {
    switch (layer) {
        case RenderLayer::BACKGROUND:
            // Render panel backgrounds
            for (const auto& panel : panels) {
                renderPanel(panel);
            }
            break;
            
        case RenderLayer::HUD_ELEMENTS:
            // Render image elements
            for (const auto& element : imageElements) {
                if (element.is3D) {
                    renderImage3D(element);
                } else {
                    // Create model matrix for scaling and translation with Z depth
                    imageShader->use();
                    
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, element.position);  // Use full 3D position including Z
                    model = glm::scale(model, glm::vec3(element.size.x, element.size.y, 1.0f));
                    
                    imageShader->setMat4("model", model);
                    
                    // Enable blending (depth testing managed at frame level)
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    
                    // Bind texture
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, element.texture->getGLid());
                    
                    // Draw quad
                    glBindVertexArray(imageVAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                    
                    glDisable(GL_BLEND);
                }
            }
            break;
            
        case RenderLayer::TEXT_OVERLAY:
            // Render text overlays
            for (const auto& overlay : textOverlays) {
                if (overlay.is3D) {
                    renderText3D(overlay);
                } else {
                    // Text rendering (depth testing managed at frame level)
                    
                    // Activate corresponding render state
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    textShader->use();
                    textShader->setVec3("textColor", overlay.color);
                    glActiveTexture(GL_TEXTURE0);
                    glBindVertexArray(VAO);

                    // Iterate through all characters
                    float x = overlay.position.x;
                    float z = overlay.position.z;  // Use Z from position
                    for (char c : overlay.text) {
                        Character ch = Characters[c];

                        float xpos = x + ch.Bearing.x * overlay.scale;
                        float ypos = overlay.position.y - (ch.Size.y - ch.Bearing.y) * overlay.scale;

                        float w = ch.Size.x * overlay.scale;
                        float h = ch.Size.y * overlay.scale;
                        // Update VBO for each character with Z coordinate
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
                        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                        x += (ch.Advance >> 6) * overlay.scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
                    }
                    glBindVertexArray(0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_BLEND);
                }
            }
            break;
            
        case RenderLayer::CURSOR:
            // Render cursor elements with Z depth
            for (const auto& element : cursorElements) {
                // Create model matrix for scaling and translation with Z depth
                imageShader->use();
                
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, element.position);  // Use full 3D position including Z
                model = glm::scale(model, glm::vec3(element.size.x, element.size.y, 1.0f));
                
                imageShader->setMat4("model", model);
                
                // Enable blending (depth testing managed at frame level)
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                // Bind texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, element.texture->getGLid());
                
                // Draw quad
                glBindVertexArray(imageVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                
                glDisable(GL_BLEND);
            }
            break;
    }
}

// Internal 3D rendering methods
void Interface::renderText3D(const TextOverlay& textOverlay) {
    setupBlending();
    
    text3DShader->use();
    text3DShader->setMat4("projection", currentProjection);
    text3DShader->setMat4("view", currentView);
    text3DShader->setVec3("textColor", textOverlay.color);
    
    glm::mat4 model = glm::mat4(1.0f);
    
    // Apply panel transformation if attached to a panel
    if (textOverlay.panelId >= 0 && textOverlay.panelId < panels.size()) {
        model = getPanelMatrix(panels[textOverlay.panelId]);
    }
    
    // Apply text position and scale
    model = glm::translate(model, textOverlay.position);
    model = glm::scale(model, glm::vec3(textOverlay.scale));
    
    text3DShader->setMat4("model", model);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    
    // Render each character
    float x = 0.0f;
    for (char c : textOverlay.text) {
        Character ch = Characters[c];
        
        float xpos = x + ch.Bearing.x * textOverlay.scale;
        float ypos = -(ch.Size.y - ch.Bearing.y) * textOverlay.scale;
        
        float w = ch.Size.x * textOverlay.scale;
        float h = ch.Size.y * textOverlay.scale;
        
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
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += (ch.Advance >> 6) * textOverlay.scale;
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    disableBlending();
}

void Interface::renderImage3D(const ImageElement& imageElement) {
    setupBlending();
    
    image3DShader->use();
    image3DShader->setMat4("projection", currentProjection);
    image3DShader->setMat4("view", currentView);
    
    glm::mat4 model = glm::mat4(1.0f);
    
    // Apply panel transformation if attached to a panel
    if (imageElement.panelId >= 0 && imageElement.panelId < panels.size()) {
        model = getPanelMatrix(panels[imageElement.panelId]);
    }
    
    // Apply image position and scale
    model = glm::translate(model, imageElement.position);
    model = glm::scale(model, glm::vec3(imageElement.size.x, imageElement.size.y, 1.0f));
    
    image3DShader->setMat4("model", model);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imageElement.texture->getGLid());
    
    // Draw quad
    glBindVertexArray(imageVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    disableBlending();
}

void Interface::renderPanel(const HudPanel& panel) {
    if (!panel.backgroundTexture) return;
    
    setupBlending();
    
    image3DShader->use();
    image3DShader->setMat4("projection", currentProjection);
    image3DShader->setMat4("view", currentView);
    
    glm::mat4 model = getPanelMatrix(panel);
    image3DShader->setMat4("model", model);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, panel.backgroundTexture->getGLid());
    
    // Draw quad
    glBindVertexArray(imageVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    disableBlending();
}

// Helper methods
glm::mat4 Interface::getPanelMatrix(const HudPanel& panel) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, panel.position);
    
    // Apply rotations in order: Z, Y, X (standard Euler angles)
    model = glm::rotate(model, glm::radians(panel.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(panel.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(panel.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    
    model = glm::scale(model, glm::vec3(panel.size.x, panel.size.y, 1.0f));
    
    return model;
}

void Interface::setupBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Interface::disableBlending() {
    glDisable(GL_BLEND);
}

Interface::~Interface()
{
    // Clean up FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &imageVAO);
    glDeleteBuffers(1, &imageVBO);
    
    // Clean up shaders
    delete textShader;
    delete imageShader;
    delete text3DShader;
    delete image3DShader;
}
