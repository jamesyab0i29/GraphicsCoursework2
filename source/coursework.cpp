#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Function prototypes
void keyboardInput(GLFWwindow *window);
void mouseInput(GLFWwindow* window);

// Create camera object
Camera camera(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -2.0f));

//object structure
struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle = 0.0f;
    std::string name;
};

//frame timer
float previousTime = 0.0f;
float deltaTime = 0.0f;

// Light struct
struct Light
{
    glm::vec3 position;
    glm::vec3 colour;
    float constant;
    float linear;
    float quadratic;
    unsigned int type;
    glm::vec3 direction;
    float cosPhi;
};

// Create vector of light sources
std::vector<Light> lightSources;

int main( void )
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Computer Graphics Coursework", NULL, NULL);
    
    if( window == NULL ){
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    //enable backface culling
    glEnable(GL_CULL_FACE);
   

    // Compile shader programs
    unsigned int shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");


    //load model
    Model teapot("../assets/teapot.obj");
    Model sphere("../assets/sphere.obj");
    Model cube("../assets/cube.obj");

    //load textures
    teapot.addTexture("../assets/blue.bmp", "diffuse");
    teapot.addTexture("../assets/diamond_normal.png", "normal");
    cube.addTexture("../assets/neutral_normal.png", "normal");
    cube.addTexture("../assets/crate.png", "diffuse");

    //use shaders
    glUseProgram(shaderID);

    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    //mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    //cube pos
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(-3.0f, 0.0f, 0.0f),
    };
    //teapot pos
    glm::vec3 teapotPositions[] = {
        glm::vec3(0.0f, 1.0f, 0.0f),
    };
    
    
    //cube to object vecotr
    std::vector<Object> objects;
    Object object;
    object.name = "cube";
    for (unsigned int i = 0; i < 3; i++)
    {
        object.position = cubePositions[i];
        object.rotation = glm::vec3(0.0f, 10.0f, 0.0f);
        object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
        object.angle = Maths::radians(90.0f * i);
        objects.push_back(object);
    };

    //teapot vector
    object.name = "teapot";
    for (unsigned int i = 0; i < 1; i++)
    {
        object.position = teapotPositions[i];
        object.rotation = glm::vec3(0.0f, 10.0f, 0.0f);
        object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
        object.angle = Maths::radians(90.0f * i);
        objects.push_back(object);
    };

    // Load a floor model for the floor and add textures
    Model floor("../assets/plane.obj");
    floor.addTexture("../assets/stones_diffuse.png", "diffuse");
    floor.addTexture("../assets/stones_normal.png", "normal");

    //wall model
    Model wall("../assets/plane.obj");
    wall.addTexture("../assets/bricks_diffuse.png", "diffuse");
    wall.addTexture("../assets/bricks_normal.png", "normal");
    

    // Define floor light properties
    floor.ka = 0.2f;
    floor.kd = 1.0f;
    floor.ks = 1.0f;
    floor.Ns = 20.0f;

    wall.ka = 0.2f;
    wall.kd = 1.0f;
    wall.ks = 1.0f;
    wall.Ns = 20.0f;

    // Add floor model to objects vector
    object.position = glm::vec3(0.0f, -0.5f, 0.0f);
    object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = 0.0f;
    object.name = "floor";
    objects.push_back(object);
    object.position = glm::vec3(0.0f, 3.0f, 0.0f);
    object.angle = Maths::radians(180.0f);
    objects.push_back(object);

    // Add wall model to objects vector
    object.position = glm::vec3(0.0f, 0.0f, -10.0f);
    object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = Maths::radians(90.0f);
    object.name = "wall";
    objects.push_back(object);

    object.position = glm::vec3(0.0f, 0.0f, 10.0f);
    object.rotation = glm::vec3(-1.0f, 0.0f, 0.0f);
    objects.push_back(object);

    object.position = glm::vec3(10.0f, 0.0f, 0.0f);
    object.rotation = glm::vec3(0.0f, 0.0f, 1.0f);
    objects.push_back(object);

    object.position = glm::vec3(-10.0f, 0.0f, 0.0f);
    object.rotation = glm::vec3(0.0f, 0.0f, -1.0f);
    objects.push_back(object);
    
    
    
    //object lighting
    teapot.ka = 0.2f;
    teapot.kd = 0.7f;
    teapot.ks = 10.0f;
    teapot.Ns = 20.f;
    cube.ka = 0.2f;
    cube.kd = 0.7f;
    cube.ks = 10.0f;
    cube.Ns = 20.f;
    float constant = 1.0f;
    float linear = 0.1f;
    float quadratic = 0.02f;

    // Add first point light source
    Light light;
    light.position = glm::vec3(5.0f, 3.0f, 5.0f);
    light.colour = glm::vec3(1.0f, 1.0f, 1.0f);
    light.constant = 1.0f;
    light.linear = 0.1f;
    light.quadratic = 0.02f;
    light.type = 1;
    lightSources.push_back(light);

    // Add second point light source
    light.position = glm::vec3(-5.0f, 3.0f, -5.0f);
    lightSources.push_back(light);

    // Add third point light source
    light.position = glm::vec3(5.0f, 3.0f, -5.0f);
    lightSources.push_back(light);

    // Add fourth point light source
    light.position = glm::vec3(-5.0f, 3.0f, 5.0f);
    lightSources.push_back(light);

    // Add spotlight
    light.position = glm::vec3(0.0f, 3.0f, 0.0f);
    light.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    light.colour = glm::vec3(10.0f, 1.0f, 1.0f);
    light.cosPhi = std::cos(Maths::radians(45.0f));
    light.type = 2;
    lightSources.push_back(light);

    // Add directional light
    light.direction = glm::vec3(1.0f, -1.0f, 0.0f);
    light.colour = glm::vec3(1.0f, 1.0f, 0.0f);
    light.type = 3;
    //lightSources.push_back(light);



    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        //update timer
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        // Get inputs
        keyboardInput(window);
        mouseInput(window);

        
        // Clear the window
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //calculate view
        camera.calculateMatrices();

        // Send multiple light source properties to the shader
        for (unsigned int i = 0; i < static_cast<unsigned int>(lightSources.size()); i++)
        {
            glm::vec3 viewSpaceLightPosition = glm::vec3(camera.view * glm::vec4(lightSources[i].position, 1.0f));
            std::string idx = std::to_string(i);
            glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].colour").c_str()), 1, &lightSources[i].colour[0]);
            glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].position").c_str()), 1, &viewSpaceLightPosition[0]);
            glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].constant").c_str()), lightSources[i].constant);
            glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].linear").c_str()), lightSources[i].linear);
            glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].quadratic").c_str()), lightSources[i].quadratic);
            glUniform1i(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].type").c_str()), lightSources[i].type);
            glm::vec3 viewSpaceLightDirection = glm::vec3(camera.view * glm::vec4(lightSources[i].direction, 0.0f));
            glUniform3fv(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].direction").c_str()), 1, &viewSpaceLightDirection[0]);
            glUniform1f(glGetUniformLocation(shaderID, ("lightSources[" + idx + "].cosPhi").c_str()), lightSources[i].cosPhi);
        }

        // Send object lighting properties to the fragment shader
        glUniform1f(glGetUniformLocation(shaderID, "ka"), teapot.ka);
        glUniform1f(glGetUniformLocation(shaderID, "kd"), teapot.kd);
        glUniform1f(glGetUniformLocation(shaderID, "ks"), teapot.ks);
        glUniform1f(glGetUniformLocation(shaderID, "Ns"), teapot.Ns);


        //loop through objects
        for (int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            //calculate model
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            glm::mat4 model = translate * rotate * scale;

            //send mvp to vertex shader
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            // Draw the model
            if (objects[i].name == "teapot")
                teapot.draw(shaderID);

            if (objects[i].name == "cube")
                cube.draw(shaderID);

            if (objects[i].name == "floor")
                floor.draw(shaderID);

            if (objects[i].name == "wall")
                wall.draw(shaderID);

        };
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        for (unsigned int i = 0; i < static_cast<unsigned int>(lightSources.size()); i++)
        {
            // Calculate model matrix
            glm::mat4 translate = Maths::translate(lightSources[i].position);
            glm::mat4 scale = Maths::scale(glm::vec3(0.1f));
            glm::mat4 model = translate * scale;

            // Send the MVP and MV matrices to the vertex shader
            glm::mat4 MVP = camera.projection * camera.view * model;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

            // Send model, view, projection matrices and light colour to light shader
            glUniform3fv(glGetUniformLocation(shaderID, "lightColour"), 1, &lightSources[i].colour[0]);

            // Draw light source
            sphere.draw(shaderID);
        }
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    // Cleanup
    glDeleteProgram(shaderID);

    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // Move the camera using WSAD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.right;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.right;
}

void mouseInput(GLFWwindow* window)
{
    // Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Update yaw and pitch angles
    camera.yaw += 0.0005f * float(xPos - 1024 / 2);
    camera.pitch += 0.0005f * float(768 / 2 - yPos);
}
