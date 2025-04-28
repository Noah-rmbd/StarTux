#ifndef SHAPE_MODEL_H
#define SHAPE_MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "shader.h"
#include "shape.h"

class ShapeModel : public Shape{
    public:
        ShapeModel(const std::string& filepath, Shader* shader_program);
        
        void draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection) override;
        void key_handler(int key);
        virtual ~ShapeModel();

    private:
        void loadModel(const std::string& filepath);
        void processNode(aiNode* node, const aiScene* scene);
        void processMesh(aiMesh* mesh);
        void setupMesh();

        // Phong shader uniforms
        GLint light_pos_loc;
        GLint light_color_loc;
        GLint object_color_loc;
        glm::vec3 light_position;
        glm::vec3 light_color;
        glm::vec3 object_color;

        unsigned int VAO; // Vertex Array Object
        unsigned int VBO; // Vertex Buffer Object
        unsigned int EBO; // Element Buffer Object
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        unsigned int indexCount;
};

#endif