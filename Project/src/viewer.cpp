#include "viewer.h"
#include <filesystem>
#include <iostream>

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <glm/gtc/matrix_transform.hpp>

Viewer::Viewer(int width, int height) : windowWidth(width), windowHeight(height)
{
    if (!glfwInit())    // initialize window system glfw
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
    
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
    }

    // Set user pointer for GLFW window to this Viewer instance
    glfwSetWindowUserPointer(win, this);

    // register event handlers
    glfwSetKeyCallback(win, key_callback_static);
    glfwSetCursorPosCallback(win, mouse_callback_static); // mouse inputs
    glfwSetMouseButtonCallback(win, mouse_button_callback_static);
    //glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor and lock it

    // useful message to check OpenGL renderer characteristics
    std::cout << glGetString(GL_VERSION) << ", GLSL "
              << glGetString(GL_SHADING_LANGUAGE_VERSION) << ", Renderer "
              << glGetString(GL_RENDERER) << std::endl;

    // initialize GL by setting viewport and default render characteristics
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    /* tell GL to only draw onto a pixel if the shape is closer to the viewer
    than anything already drawn at that pixel */
    glEnable( GL_DEPTH_TEST ); /* enable depth-testing */
    /* with LESS depth-testing interprets a smaller depth value as meaning "closer" */
    glDepthFunc( GL_LESS );

    // Initialize the game
    game = new Game();
    
    // Initialize interface
    startup_screen = new StartupScreen(windowWidth, windowHeight);
}

Viewer::~Viewer()
{ 
    // Clean up game and window
    delete game;
    //delete interface;
    delete startup_screen;
    glfwDestroyWindow(win);
    glfwTerminate();
}

void Viewer::run()
{   
    const float targetFrameTime = 1.0f / 60.0f; // Target frame time for 60 FPS
    Interface* startUpScreen = new Interface();

    // Main render loop for this OpenGL window
    while (!glfwWindowShouldClose(win))
    {   
        auto frameStart = std::chrono::high_resolution_clock::now(); // Stores the start of the frame

        // clear draw buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(startGame) {
            camera.keyboard_events(keyStates);
            game->keyHandler(keyStates, glfwGetTime());
            game->updateGame();

            if(!camera.devMode){
                camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
                camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
                camera.cameraPos = game->player->position + glm::vec3(0.0f, 0.05f, -0.22f);
                //camera.cameraPos = game->player->position + glm::vec3(0.0f, 0.1f, -0.52f);
            }

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

            game->scene_root->draw(model, view, projection);

            // Render HUD
            //interface->renderText("Score: " + std::to_string(400), 25.0f, 960.0f - 50.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            //interface->renderText("Lives: " + std::to_string(3), 25.0f, 960.0f - 100.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        else {
            // Render start screen
            // Center the image and scale it to fit the window
            //float imageWidth = 337.0f;  // Adjust these values to fit your needs
            float imageWidth = 320.0f;
            float imageHeight = 180.0f;
            float centerX = (windowWidth - imageWidth) / 2.0f;
            float centerY = (windowHeight - imageHeight) / 2.0f;
            
            startup_screen->update();
            if (startup_screen->isLaunched()) {
                startGame = true;
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor and lock it
            }
        }

        // Calculate and render FPS
        static float lastTime = 0.0f;
        static int frameCount = 0;
        static float fps = 0.0f;
        
        float currentTime = glfwGetTime();
        frameCount++;
        
        if (currentTime - lastTime >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }
        
        //interface->renderText("FPS: " + std::to_string(static_cast<int>(fps)), 1280.0f - 150.0f, 960.0f - 50.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Poll for and process events
        glfwPollEvents();

        // flush render commands, and swap draw buffers
        glfwSwapBuffers(win);

        // Calculate frame time and sleep if necessary
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> frameDuration = frameEnd - frameStart;
        float frameTime = frameDuration.count();

        if (frameTime < targetFrameTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((targetFrameTime - frameTime) * 1000)));
        }
    }
}

void Viewer::key_callback_static(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    viewer->on_key(key, action);
}

void Viewer::mouse_callback_static(GLFWwindow* window, double xpos, double ypos)
{
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    viewer->camera.mouse_callback(xpos, ypos);
}

void Viewer::mouse_button_callback_static(GLFWwindow* window, int button, int action, int mods)
{
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    viewer->on_mouse_button(button, action, xpos, ypos);
}

void Viewer::on_key(int key, int action)
{
    // 'Q' or 'Escape' quits
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }

    // Store key state (true when pressed, false when released)
    if (action == GLFW_PRESS) {
        if(!keyStates[key].first){
            keyStates[key].second = glfwGetTime(); // store the time when the key started to be pressed
            keyStates[key].first = true; 
        }
    } 
    else if (action == GLFW_RELEASE) {
        keyStates[key].first = false;
    }
}

void Viewer::on_mouse_button(int button, int action, double xpos, double ypos)
{
    if (!startGame) {
        startup_screen->mouse(button, action, xpos, ypos);
    }
}
