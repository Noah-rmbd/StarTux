#pragma once

#include <GL/glew.h>
#include <utility>

/**
 * RAII wrapper for OpenGL textures
 * Automatically creates and destroys OpenGL texture resources
 */
class GLTexture {
private:
    GLuint textureId = 0;
    
public:
    // Constructor - automatically creates texture
    GLTexture() {
        glGenTextures(1, &textureId);
    }
    
    // Destructor - automatically cleans up
    ~GLTexture() {
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }
    }
    
    // Make it non-copyable (prevents double-deletion)
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    
    // Make it moveable (transfer ownership) - C++11 compatible
    GLTexture(GLTexture&& other) noexcept 
        : textureId(other.textureId) {
        other.textureId = 0;  // Reset other
    }
    
    GLTexture& operator=(GLTexture&& other) noexcept {
        if (this != &other) {
            // Clean up our current texture
            if (textureId != 0) {
                glDeleteTextures(1, &textureId);
            }
            // Take ownership of other's texture
            textureId = other.textureId;
            other.textureId = 0;  // Reset other
        }
        return *this;
    }
    
    // Get the OpenGL handle
    GLuint GetHandle() const { return textureId; }
    
    // Check if valid
    bool IsValid() const { return textureId != 0; }
    
    // Bind the texture
    void Bind() const {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
    
    // Unbind
    static void Unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

/**
 * RAII wrapper for OpenGL buffers (VBO, EBO, etc.)
 */
class GLBuffer {
private:
    GLuint bufferId = 0;
    GLenum bufferType;
    
public:
    explicit GLBuffer(GLenum type = GL_ARRAY_BUFFER) : bufferType(type) {
        glGenBuffers(1, &bufferId);
    }
    
    ~GLBuffer() {
        if (bufferId != 0) {
            glDeleteBuffers(1, &bufferId);
        }
    }
    
    // Non-copyable, moveable
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    
    GLBuffer(GLBuffer&& other) noexcept 
        : bufferId(other.bufferId), bufferType(other.bufferType) {
        other.bufferId = 0;  // Reset other
    }
    
    GLBuffer& operator=(GLBuffer&& other) noexcept {
        if (this != &other) {
            if (bufferId != 0) {
                glDeleteBuffers(1, &bufferId);
            }
            bufferId = other.bufferId;
            bufferType = other.bufferType;
            other.bufferId = 0;  // Reset other
        }
        return *this;
    }
    
    GLuint GetHandle() const { return bufferId; }
    bool IsValid() const { return bufferId != 0; }
    
    void Bind() const {
        glBindBuffer(bufferType, bufferId);
    }
    
    void Unbind() const {
        glBindBuffer(bufferType, 0);
    }
    
    // Upload data to buffer
    void SetData(const void* data, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW) {
        Bind();
        glBufferData(bufferType, size, data, usage);
    }
};

/**
 * RAII wrapper for OpenGL Vertex Array Objects
 */
class GLVAO {
private:
    GLuint vaoId = 0;
    
public:
    GLVAO() {
        glGenVertexArrays(1, &vaoId);
    }
    
    ~GLVAO() {
        if (vaoId != 0) {
            glDeleteVertexArrays(1, &vaoId);
        }
    }
    
    // Non-copyable, moveable
    GLVAO(const GLVAO&) = delete;
    GLVAO& operator=(const GLVAO&) = delete;
    
    GLVAO(GLVAO&& other) noexcept 
        : vaoId(other.vaoId) {
        other.vaoId = 0;  // Reset other
    }
    
    GLVAO& operator=(GLVAO&& other) noexcept {
        if (this != &other) {
            if (vaoId != 0) {
                glDeleteVertexArrays(1, &vaoId);
            }
            vaoId = other.vaoId;
            other.vaoId = 0;  // Reset other
        }
        return *this;
    }
    
    GLuint GetHandle() const { return vaoId; }
    bool IsValid() const { return vaoId != 0; }
    
    void Bind() const {
        glBindVertexArray(vaoId);
    }
    
    static void Unbind() {
        glBindVertexArray(0);
    }
};