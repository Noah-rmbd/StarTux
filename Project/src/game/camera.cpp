#include "camera.h"

Camera::Camera()
{
    // camera attributes
    cameraPos = glm::vec3(0.0f, 0.0f, 5.0f); // Initial position
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Direction the camera is facing
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up direction
    cameraSpeed = 0.01f; // Adjust movement speed
}

void Camera::mouse_callback(double xpos, double ypos){
    if(devMode){
        if (this->firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Inverted since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Limit pitch to prevent flipping the camera
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // Update camera direction
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFront = glm::normalize(front);
    }
}

void Camera::keyboard_events(std::unordered_map<int, std::pair<bool, double>> keyStates){
    // Camera Movement
    if (keyStates[GLFW_KEY_W].first) // Move Forward
        this->cameraPos += this->cameraSpeed * this->cameraFront;
            
    if (keyStates[GLFW_KEY_S].first) // Move Backward
        this->cameraPos -= this->cameraSpeed * this->cameraFront;
            
    if (keyStates[GLFW_KEY_A].first) // Move Left
        this->cameraPos -= glm::normalize(glm::cross(this->cameraFront, this->cameraUp)) * this->cameraSpeed;
            
    if (keyStates[GLFW_KEY_D].first) // Move Right
        this->cameraPos += glm::normalize(glm::cross(this->cameraFront, this->cameraUp)) * this->cameraSpeed;
            
    if (keyStates[GLFW_KEY_R].first) { // Move Up
        this->cameraPos += this->cameraSpeed * this->cameraUp;   
    }
    if (keyStates[GLFW_KEY_F].first) { // Move Down
        this->cameraPos -= this->cameraSpeed * this->cameraUp; 
    }
    if (keyStates[GLFW_KEY_X].first) { // Move Down
        this->devMode = false;
    }
    if (keyStates[GLFW_KEY_C].first) { // Move Down
        this->devMode = true; 
    }
    
}

 //void Camera::updateAngle(){
    //cameraFront = glm::vec3(0.0f, cos(yAngle), sin(yAngle));
 //}