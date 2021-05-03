#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <assimp/ai_assert.h>
#include "Enemy.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
unsigned int createTexture(const char* filename);
void setupDirLight(Shader* shader, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
void setupSpotLight(Shader* shader, float cutOff, float outerCutOff, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
void setupPointLight(Shader* shader, unsigned int n, glm::vec3 position, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
void drawReticle(Shader* reticleShader, unsigned int VAO);
void renderScene(Shader* shader, bool renderExtras);
void update();

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// combat
bool shooting = false;
float lastShot = 0.0f;

// rectangle for crosshair
float vertices[] = {
    -0.5f, -0.5f, 0.0f, // bottom left
    0.5f, -0.5f, 0.0f, // bottom right
    0.5f, 0.5f, 0.0f, // top right
    -0.5f, 0.5f, 0.0f // top left
};
unsigned int indices[] = {
    0, 1, 2,
    0, 2, 3
};

// shaders
Shader* ourShader;
Shader* lightSourceShader;
Shader* reticleShader;
Shader* depthShader;

// models
Model* hallway;
Model* gun;
Model* light;

// textures
unsigned int red;
unsigned int redSpec;
unsigned int texture;
unsigned int textureSpec;
unsigned int chip;
unsigned int chipSpec;

Enemy* enemies[5];

unsigned int VAO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FPS", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    ourShader = new Shader("shaders/lighter.vert", "shaders/lighter.frag");
    lightSourceShader = new Shader("shaders/lightsource.vert", "shaders/lightsource.frag");
    reticleShader = new Shader("shaders/reticle.vert", "shaders/reticle.frag");
    depthShader = new Shader("shaders/depth.vert", "shaders/depth.frag", "shaders/depth.geom");

    // load textures
    red = createTexture("Images/paintedmetal/PaintedMetal006_2K_Color.jpg");
    redSpec = createTexture("Images/paintedmetal/PaintedMetal006_2K_Metalness.jpg");
    texture = createTexture("Images/metal_diffuse.jpg");
    textureSpec = createTexture("Images/metal_specular.jpg");
    chip = createTexture("Images/chip/Chip001_2K_Color.jpg");
    chipSpec = createTexture("Images/chip/Chip001_2K_Metalness.jpg");

    // load models
    // -----------
    //Model ourModel("Models/backpack/backpack.obj");
    hallway = new Model("Models/hallway/uploads_files_1892077_Corridor.obj");
    gun = new Model("Models/gun/Cyborg_Weapon.obj");
    light = new Model("Models/light/untitled.obj");

    // setup crosshair traingles
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // shadows
    // generate shadow buffer
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::vec3 lightPos = glm::vec3(0.0f, 2.2f, 10.0f);

    // shader configuration
    ourShader->use();
    // setup lighting
    //setupSpotLight(ourShader, 12.5f, 17.5f, 1.0f, 0.05f, 0.01f, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    setupPointLight(ourShader, 0, lightPos, 1.0f, 0.09f, 0.032f, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    //setupPointLight(ourShader, 1, glm::vec3(0.0f, 2.2f, 0.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    //setupPointLight(ourShader, 2, glm::vec3(0.0f, 2.2f, 10.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    //setupPointLight(ourShader, 3, glm::vec3(0.0f, 2.2f, 20.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader->setInt("depthMap", 1);

    for (int i = 0; i < 5; i++) {
        enemies[i] = new Enemy(glm::vec3(0.0f, 0.0f, 0.0f + i * 5), "Models/robot/uploads_files_985353_BattleBot.obj", 0, red, redSpec);
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        //printf("%f %f %f\n", camera.Position.x, camera.Position.y, camera.Position.z);
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);
        update(); // next frame of gameplay, not render

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // 1. first render to depth map
        // ConfigureShaderAndMatrices - render scene from lights perspective
        /*float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 1.0f, 15.0f),
            glm::vec3(-3.0f, 1.0f, 15.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;*/
        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        float near = 1.0f;
        float far = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

        

        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj *
            glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader->use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader->setFloat("far_plane", far);
        depthShader->setVec3("lightPos", lightPos);
        renderScene(depthShader, false);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);




        // 2. then render scene as normal with shadow mapping (using depth map)
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //ConfigureShaderAndMatrices();

        ourShader->use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader->setMat4("projection", projection);
        ourShader->setMat4("view", view);
        ourShader->setVec3("viewPos", camera.Position);
        ourShader->setFloat("far_plane", far);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(ourShader, true);

        // draw reticle
        reticleShader->use();
        drawReticle(reticleShader, VAO);

        // draw reticle
        drawReticle(reticleShader, VAO);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void update()
{
    for (Enemy* bot : enemies) {
        if (bot->alive) {
            bot->move(deltaTime);
        }
    }

    // combat
    if (shooting)
    {
        for (Enemy* bot : enemies) {
            if (bot->playerLooking(camera.Position, camera.Front)) {
                shooting = false;
                lastShot = glfwGetTime();
                bot->takeDamage();
            }
        }
    }
}

void renderScene(Shader* shader, bool renderExtras)
{
    //shader->use();

    

    // torch position
    //shader->setVec3("spotLight.position", camera.Position);
    //shader->setVec3("spotLight.direction", camera.Front);

    // use metal texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureSpec);

    // render hallway
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setMat4("model", model);
    if (renderExtras)
    {
        glDisable(GL_CULL_FACE);
        shader->setInt("reverse_normals", 1);
        hallway->Draw(*shader);  
        shader->setInt("reverse_normals", 0);
    }
    else
    {
        hallway->Draw(*shader);
    }

    // draw enemies
    for (Enemy* bot : enemies) {
        if (bot->alive) {
            bot->draw(*shader);
        }
    }

    if (renderExtras)
    {
        // draw physical light
        lightSourceShader->use();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightSourceShader->setMat4("projection", projection);
        lightSourceShader->setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f, 4.0f, 10.0f));
        model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.001f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        lightSourceShader->setMat4("model", model);
        light->Draw(*lightSourceShader);

        // clear depth buffer so gun and reticle are rendered above world
        glClear(GL_DEPTH_BUFFER_BIT);

        // use chip texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, chip);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, chipSpec);

        // render gun
        shader->use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, camera.Position);
        model = glm::rotate(model, glm::radians(-camera.Yaw - 90), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.1f, -0.3f, -0.4f)); // rotate around the player if the player was at the origin
        shader->setMat4("model", model);
        gun->Draw(*shader);
    }

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetMouseButton(window, 0) == GLFW_PRESS && glm::abs(lastShot - glfwGetTime()) > 0.3f) {
        shooting = true;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int createTexture(const char* filename)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void setupDirLight(Shader* shader, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
    shader->setVec3("dirLight.direction", direction);
    shader->setVec3("dirLight.ambient", ambient);
    shader->setVec3("dirLight.diffuse", diffuse);
    shader->setVec3("dirLight.specular", specular);
}

void setupSpotLight(Shader* shader, float cutOff, float outerCutOff, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
    shader->setFloat("spotLight.cutOff", glm::cos(glm::radians(cutOff)));
    shader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));
    shader->setFloat("spotLight.constant", constant);
    shader->setFloat("spotLight.linear", linear);
    shader->setFloat("spotLight.quadratic", quadratic);
    shader->setVec3("spotLight.ambient", ambient);
    shader->setVec3("spotLight.diffuse", diffuse); // darken diffuse light a bit
    shader->setVec3("spotLight.specular", specular);
}

void setupPointLight(Shader* shader, unsigned int n, glm::vec3 position, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
    char text[64];
    sprintf_s(text, "pointLights[%d].position", n);
    shader->setVec3(text, position);
    sprintf_s(text, "pointLights[%d].constant", n);
    shader->setFloat(text, constant);
    sprintf_s(text, "pointLights[%d].linear", n);
    shader->setFloat(text, linear);
    sprintf_s(text, "pointLights[%d].quadratic", n);
    shader->setFloat(text, quadratic);
    sprintf_s(text, "pointLights[%d].ambient", n);
    shader->setVec3(text, ambient);
    sprintf_s(text, "pointLights[%d].diffuse", n);
    shader->setVec3(text, diffuse);
    sprintf_s(text, "pointLights[%d].specular", n);
    shader->setVec3(text, specular);
}

void drawReticle(Shader* reticleShader, unsigned int VAO) {
    reticleShader->use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.01f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    reticleShader->setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.01f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    reticleShader->setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0f));
    model = glm::scale(model, glm::vec3(0.005f, 0.01f, 0.01f));
    reticleShader->setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.01f, 0.0f));
    model = glm::scale(model, glm::vec3(0.005f, 0.01f, 0.01f));
    reticleShader->setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}