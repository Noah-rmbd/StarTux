#include "shape_model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

// Debug
#include <glm/gtx/string_cast.hpp>

ShapeModel::ShapeModel(const std::string &filepath, Shader *shader_program)
    : Shape(shader_program), VAO(0), VBO(0), EBO(0), indexCount(0) {
  // Get uniform locations for phong shader
  light_pos_loc = glGetUniformLocation(this->shader_program_, "lightPos");
  light_color_loc = glGetUniformLocation(this->shader_program_, "lightColor");
  object_color_loc = glGetUniformLocation(this->shader_program_, "objectColor");

  // Set default values for phong lighting
  light_position = glm::vec3(1.0f, 1.0f, 1.0f);
  light_color = glm::vec3(1.0f, 1.0f, 1.0f);
  object_color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue color

  loadModel(filepath);
}

void ShapeModel::loadModel(const std::string &filepath) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    return;
  }

  processNode(scene->mRootNode, scene);
  setupMesh();
}

void ShapeModel::processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

void ShapeModel::processMesh(aiMesh *mesh) {
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    vertices.push_back(mesh->mVertices[i].x);
    vertices.push_back(mesh->mVertices[i].y);
    vertices.push_back(mesh->mVertices[i].z);

    if (mesh->mNormals) {
      vertices.push_back(mesh->mNormals[i].x);
      vertices.push_back(mesh->mNormals[i].y);
      vertices.push_back(mesh->mNormals[i].z);
    }

    if (mesh->mTextureCoords[0]) {
      vertices.push_back(mesh->mTextureCoords[0][i].x);
      vertices.push_back(mesh->mTextureCoords[0][i].y);
    }
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  indexCount += mesh->mNumFaces * 3;
}

void ShapeModel::setupMesh() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // Vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

  // Normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  // Texture coordinates
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));

  glBindVertexArray(0);
}

void ShapeModel::draw(glm::mat4 &model, glm::mat4 &view,
                      glm::mat4 &projection) {
  // Teste
  // std::cout << "Drawing asteroid at " << glm::to_string(model) << std::endl;
  glUseProgram(this->shader_program_);
  glBindVertexArray(VAO);

  Shape::draw(model, view, projection);

  // If we have a texture, bind it
  if (texture_) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_->getGLid());
    glUniform1i(glGetUniformLocation(shader_program_, "texture1"), 0);
  } else {
    // Set phong shader uniforms if no texture
    glUniform3fv(light_pos_loc, 1, glm::value_ptr(light_position));
    glUniform3fv(light_color_loc, 1, glm::value_ptr(light_color));
    glUniform3fv(object_color_loc, 1, glm::value_ptr(object_color));
  }

  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

ShapeModel::~ShapeModel() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

void ShapeModel::key_handler(int key) {
  // Handle key events if needed
}

ShapeModel::ShapeModel(const ShapeModel &other)
    : Shape(other), // Copie les membres de base
      VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
      indexCount(other.indexCount), light_position(other.light_position),
      light_color(other.light_color), object_color(other.object_color) {
  this->shader_program_ =
      other.shader_program_; // important si Shape ne le copie pas
}

Shape *ShapeModel::clone() const { return new ShapeModel(*this); }

void ShapeModel::setTexture(Texture* texture) { texture_ = texture; }