// OpenGL_Lessen.cpp : This file contains the 'main' function. Program execution 
//beginsand ends there.
//

#include <iostream>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <list>

#include "model.h"
#include "mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utils.h"

//forward declarations
void RenderTerrain(glm::mat4 view, glm::mat4 projection); 
void RenderSkybox(glm::mat4 view, glm::mat4 projection);
void RenderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection);
void SetupResources();

glm::vec3 camPos(0, 400, 0), camFor(0, 0, 1), camUp(0, 1, 0);

unsigned int plane, planeSize, VAO, cubeSize, quadVAO, quadVBO;
unsigned int myProgram, skyProgram, modelProgram, screenProgram;

const int width = 2560;
const int height = 1440;

//Textures
unsigned int heightmapID;
unsigned int heightNormalID;
unsigned int dirtID, sandID, grassID, rockID, snowID;

unsigned int textureColorbuffer, framebuffer;

Model* backpack;
Model* campfire;
Model* tree;
Model* grass;

void handleInput(GLFWwindow* window, float deltaTime) {
    static bool w, s, a, d, space, ctrl;
    static double cursorX = -1, cursorY = -1, lastCursorX, lastCursorY;
    static float pitch, yaw;
    static float speed = 100.0f;

    float sensitivity = 100.0f * deltaTime;
    float step = speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)				w = true;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)		w = false;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)				s = true;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)		s = false;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)				a = true;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)		a = false;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)				d = true;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)		d = false;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)				space = true;
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)		space = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)		ctrl = true;
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)	ctrl = false;

    if (cursorX == -1) {
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }

    lastCursorX = cursorX;
    lastCursorY = cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);

    glm::vec2 mouseDelta(cursorX - lastCursorX, cursorY - lastCursorY);

    // TODO: calculate rotation & movement
    yaw -= mouseDelta.x * sensitivity;
    pitch += mouseDelta.y * sensitivity;

    pitch = glm::max(glm::min(pitch, 90.0f), -90.0f);
    if (yaw < -180.0f) yaw += 360;
    else if (yaw > 180.0f) yaw -= 360;

    glm::vec3 euler(glm::radians(pitch), glm::radians(yaw), 0);
    glm::quat q(euler);

    // update camera position / forward & up
    glm::vec3 translation(0, 0, 0);

    if (w) translation.z = 1;
    if (a) translation.x = 1;
    if (s) translation.z = -1;
    if (d) translation.x = -1;

    camPos += q * translation;

    camUp = q * glm::vec3(0, 1, 0);
    camFor = q * glm::vec3(0, 0, 1);
}

int main() {
    static double previousT = 0;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "No Man Sky II", nullptr,
        nullptr);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    SetupResources();

    ///END SETUP SHADER PROGRAM///

    /// Matrix Setup ///
    glUseProgram(myProgram);

    int worldLoc = glGetUniformLocation(myProgram, "world");
    int viewLoc = glGetUniformLocation(myProgram, "view");
    int projectionLoc = glGetUniformLocation(myProgram, "projection");

    /// End Matrix Setup///

    // OPENGL SETTINGS //

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    int heightmapHeight, heightmapWidth;
    int channels = 4;
    unsigned char* data;
    data = stbi_load("Heightmap2.png", &heightmapWidth, &heightmapHeight, &channels, 4);
    if (data == nullptr) {
        std::cout << "Error loading file: " << "Heightmap2.png" << std::endl;
    }
    
    glm::vec3 grassPositions[50];

    for (size_t i = 0; i < 50; i++)
    {
        grassPositions[i] = glm::vec3(rand() % heightmapWidth, 0, rand() % heightmapHeight);
        grassPositions[i].y = data[(int)(grassPositions[i].z * heightmapWidth + grassPositions[i].x) * 4] / 255.0f;
        grassPositions[i].y *= 100.0f;
    }

    while (!glfwWindowShouldClose(window)) {
        double t = glfwGetTime() * .5f;

        handleInput(window, (float)(t - previousT));
        previousT = t;

        // bind to framebuffer and draw scene as we normally would to color texture 
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(camPos, camPos + camFor, camUp);
        glm::mat4 projection = glm::perspective(glm::radians(65.0f), width / (float)height, 0.1f, 1000.0f);

        RenderSkybox(view, projection);
        RenderTerrain(view, projection);
        /*
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

        for (size_t i = 0; i < 50; i++)
        {
            RenderModel(tree, modelProgram, grassPositions[i], glm::vec3(0), .01f, view, projection);
        }

        /*glDisable(GL_BLEND);*/

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(screenProgram);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void RenderTerrain(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(myProgram);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, glm::vec3(0, 0, 0));

    glUniformMatrix4fv(glGetUniformLocation(myProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(myProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(myProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(myProgram, "camPos"), 1, glm::value_ptr(camPos));

    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(myProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightNormalID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirtID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sandID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grassID);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rockID);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, snowID);

    glBindVertexArray(plane);
    glDrawElements(GL_TRIANGLES, planeSize, GL_UNSIGNED_INT, 0);
}

void RenderSkybox(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(skyProgram);

    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);

    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, camPos);

    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(skyProgram, "camPos"), 1, glm::value_ptr(camPos)); 
    
    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(myProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, cubeSize, GL_UNSIGNED_INT, 0);
}

void SetupResources() {
    ///SETUP OBJECT///
    stbi_set_flip_vertically_on_load(true);

    backpack = new Model("backpack/backpack.obj");
    campfire = new Model("campfire/campfire.obj");
    tree = new Model("tree/tree.obj");
    grass = new Model("grass/grass.obj");

    // Vertices of our triangle!
    // need 24 vertices for normal/uv-mapped Cube
    float vertices[] = {
        // positions            //colors            // tex coords   // normals
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 0.0f,   1.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       1.f, 0.f, 0.f,

        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 2.f,       0.f, 0.f, 1.f,

        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   -1.f, 1.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   -1.f, 0.f,      -1.f, 0.f, 0.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, -1.f,      0.f, 0.f, -1.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, -1.f,      0.f, 0.f, -1.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   3.f, 0.f,       0.f, 1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   3.f, 1.f,       0.f, 1.f, 0.f,

        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,

        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       -1.f, 0.f, 0.f,

        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,

        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       0.f, 1.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       0.f, 1.f, 0.f
    };

    unsigned int indices[] = {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    cubeSize = sizeof(indices);

    unsigned int VBO, EBO, FBO, RBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    int stride = sizeof(float) * 11;    

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    // normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 8));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    ///END SETUP OBJECT///

    plane = GeneratePlane("heightmap2.png", GL_RGBA, 4, 1.0f, 1.0f, planeSize, heightmapID);

    //terrain Textures
    heightNormalID = loadTexture("NormalMap.png", GL_RGBA, 4);
    dirtID = loadTexture("dirt.jpg", GL_RGB, 3);
    sandID = loadTexture("sand.jpg", GL_RGB, 3);
    grassID = loadTexture("grass.png", GL_RGBA, 4);
    rockID = loadTexture("rock.jpg", GL_RGB, 3);
    snowID = loadTexture("snow.jpg", GL_RGB, 3);

    ///SETUP SHADER PROGRAM///

    unsigned int vertexSource, fragmentSource;
    CreateShader("vertexShader.shader", GL_VERTEX_SHADER, vertexSource);
    CreateShader("fragmentShader.shader", GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int vertSky, fragSky, vertModel, fragModel, frScreen, vsScreen;
    CreateShader("vertexShaderSky.shader", GL_VERTEX_SHADER, vertSky);
    CreateShader("fragmentShaderSky.shader", GL_FRAGMENT_SHADER, fragSky);
    CreateShader("vertModel.shader", GL_VERTEX_SHADER, vertModel);
    CreateShader("fragModel.shader", GL_FRAGMENT_SHADER, fragModel);
    CreateShader("VSScreen.shader", GL_VERTEX_SHADER, vsScreen);
    CreateShader("FSScreen.shader", GL_FRAGMENT_SHADER, frScreen);

    myProgram = glCreateProgram();
    glAttachShader(myProgram, vertexSource);
    glAttachShader(myProgram, fragmentSource);
    glLinkProgram(myProgram);

    skyProgram = glCreateProgram();
    glAttachShader(skyProgram, vertSky);
    glAttachShader(skyProgram, fragSky);
    glLinkProgram(skyProgram);

    modelProgram = glCreateProgram();
    glAttachShader(modelProgram, vertModel);
    glAttachShader(modelProgram, fragModel);
    glLinkProgram(modelProgram);

    screenProgram = glCreateProgram();
    glAttachShader(screenProgram, vsScreen);
    glAttachShader(screenProgram, frScreen);
    glLinkProgram(screenProgram);

    glDeleteShader(vertexSource);
    glDeleteShader(fragmentSource);
    glDeleteShader(vertSky);
    glDeleteShader(fragSky);
    glDeleteShader(vertModel);
    glDeleteShader(fragModel);
    glDeleteShader(vsScreen);
    glDeleteShader(frScreen);

    glUseProgram(myProgram);
    glUniform1i(glGetUniformLocation(myProgram, "heightMap"), 0);
    glUniform1i(glGetUniformLocation(myProgram, "normalMap"), 1);

    glUniform1i(glGetUniformLocation(myProgram, "dirt"), 2);
    glUniform1i(glGetUniformLocation(myProgram, "sand"), 3);
    glUniform1i(glGetUniformLocation(myProgram, "grass"), 4);
    glUniform1i(glGetUniformLocation(myProgram, "rock"), 5);
    glUniform1i(glGetUniformLocation(myProgram, "snow"), 6);
}

void RenderModel(Model * model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection) {
    // shader gebruiken
    glUseProgram(shader);

    glEnable(GL_DEPTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //world matrix bouwen
    glm::mat4 world = glm::mat4(1);
    world = glm::translate(world, position);
    world = glm::scale(world, glm::vec3(scale));
    glm::quat q(rotation);
    world *= glm::toMat4(q);

    //shader instellen
    glUniformMatrix4fv(glGetUniformLocation(shader, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shader, "camPos"), 1, glm::value_ptr(camPos));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);

    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(shader, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    model->Draw(shader);
}