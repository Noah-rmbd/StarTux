#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>



class Camera {
public:
    Camera();
    // Camera variables
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraSpeed;

    // Mouse control variables
    bool firstMouse = true;
    bool devMode = true;
    float lastX, lastY;
    float yaw = -90.0f;  // Yaw angle (left/right)
    float pitch = 0.0f;  // Pitch angle (up/down)
    float sensitivity = 0.1f;

    void mouse_callback(double xpos, double ypos);
    void keyboard_events(std::unordered_map<int, std::pair<bool, double>> keyStates);

private:
};

#endif // VIEWER_H