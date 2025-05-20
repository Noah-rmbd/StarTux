#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "shader.h"
#include "node.h"
#include "sphere.h"
#include <glm/glm.hpp>

class Projectile{
public:
    Projectile(Shader* shader, glm::vec3 position, glm::vec3 direction, glm::vec3 cursor);
    ~Projectile();
    
    Node* node;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 cursorPosition;
    float speed;
    bool active;
    double createTime;
    
    void update(double currentTime);
    void draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
    bool checkCollision(glm::vec3 asteroid_pos);
};

#endif // PROJECTILE_H