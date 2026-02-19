#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

#include <glm/gtc/matrix_transform.hpp>


#include "GLFW/glfw3.h";
#include <sstream>

using glm::vec3;
using glm::mat4;
using glm::vec4;
using glm::mat3;
using glm::radians;

float deltaTime = 0.0f;
//Last value of time change
float lastFrame = 0.0f;

//Camera sideways rotation
float cameraYaw = -90.0f;
//Camera vertical rotation
float cameraPitch = 0.0f;
//Determines if first entry of mouse into window
bool mouseFirstEntry = true;
//Positions of camera from given last frame
float cameraLastXPos = 800.0f / 2.0f;
float cameraLastYPos = 600.0f / 2.0f;



vec3 EyeCoordinates =  vec3(1.0f, 1.25f, 1.25f);
vec3 CameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 CameraUp = vec3(0.0f, 1.0f, 0.0f);


SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0),
    plane(50.0f,50.0f,1,1),
    teapot(14,glm::mat4(1.0f)),
    torus(1.75f*0.75f,1.75f*0.75f,50,50) {
   /// mesh = ObjMesh::load("../Lab 1/media/pig_triangulated.obj",true);
}
void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    model = mat4(1.0f);
    view = glm::lookAt((EyeCoordinates), CameraFront, CameraUp);
   // view = glm::lookAt(vec3(5.0f, 5.0f, 7.5f), vec3(0.0f, 0.75f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  //  model = glm::rotate(model, glm::radians(-35.0f), vec3(1.0f, 0.0f, 0.0f));
  //  model = glm::rotate(model, glm::radians(15.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = mat4(1.0f);
    angle = 0.0f;
    prog.setUniform("Spot.L", vec3(1.0f));
    prog.setUniform("Spot.La", vec3(0.05f));

    //GLuint texID = Texture::loadTexture("../Project_Template/media/texture/brick1.jpg");
    GLuint texID = Texture::loadTexture("media/texture/brick1.jpg");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);

   
    prog.setUniform("Spot.La", vec3(0.5f));
    prog.setUniform("Spot.Exponent", 50.0f);
    prog.setUniform("Spot.Cutoff", glm::radians(15.0f));


}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/NewVertexShader.vert");
        prog.compileShader("shader/NewFragmentShader.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    // Time
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    deltaTime = t - tPrev;
    if (tPrev == 0.0f)deltaTime = 0.0f;
    tPrev = t;
    angle += 0.25f * deltaTime;
    if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
    
    view = glm::lookAt(EyeCoordinates, EyeCoordinates+CameraFront, CameraUp);
}
void SceneBasic_Uniform::ProcessUserInput(int key,int action) {
    const float movementSpeed = 2.0f * deltaTime;
    
    if (action == GLFW_PRESS) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_W) {
                EyeCoordinates += movementSpeed * CameraFront;
             //   std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_A) {
                EyeCoordinates -= normalize(cross(CameraFront, CameraUp)) * movementSpeed;
               // std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_S) {
                EyeCoordinates -= movementSpeed * CameraFront;
             //   std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_D) {
                EyeCoordinates += normalize(cross(CameraFront, CameraUp)) * movementSpeed;
             //   std::cout << "Key: " << key << std::endl;
            }
        }
    }
}
void SceneBasic_Uniform::setMatrices() {
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);

    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    prog.setUniform("MVP", projection * mv);
}
void SceneBasic_Uniform::Mouse_CallBack(double Xpos, double Ypos) {
   // std::cout << "Moving mouse" << "\n";
    //Initially no last positions, so sets last positions to current positions
    if (mouseFirstEntry)
    {
        cameraLastXPos = (float)Xpos;
        cameraLastYPos = (float)Ypos;
        mouseFirstEntry = false;
    }  //Sets values for change in position since last frame to current frame
    float xOffset = (float)Xpos - cameraLastXPos;
    float yOffset = cameraLastYPos - (float)Ypos;

    //Sets last positions to current positions for next frame
    cameraLastXPos = (float)Xpos;
    cameraLastYPos = (float)Ypos;

    //Moderates the change in position based on sensitivity value
    const float sensitivity = 0.025f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    //Adjusts yaw & pitch values against changes in positions
    cameraYaw += xOffset;
    cameraPitch += yOffset;

    //Prevents turning up & down beyond 90 degrees to look backwards
    if (cameraPitch > 89.0f)
    {
        cameraPitch = 89.0f;
    }
    else if (cameraPitch < -89.0f)
    {
        cameraPitch = -89.0f;
    }

    //Modification of direction vector based on mouse turning
    vec3 direction;
    direction.x = cos(radians(cameraYaw)) * cos(radians(cameraPitch));
    direction.y = sin(radians(cameraPitch));
    direction.z = sin(radians(cameraYaw)) * cos(radians(cameraPitch));
    CameraFront = normalize(direction);
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    vec4 lightPos = vec4(10.0f*cos(angle), 10.0f, 10.0f*sin(angle), 1.0f);
    prog.setUniform("Spot.Position", vec3(view * lightPos));
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    prog.setUniform("Spot.Direction", normalMatrix*vec3(-lightPos));

    prog.setUniform("Material.Kd", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ka", vec3(0.2f*0.3f, 0.55f*0.3f, 0.9f*0.3f));  
    prog.setUniform("Material.Shininess", 100.0f);

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.5f, 0.0f));
    model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    cube.render();

   // prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
   /// prog.setUniform("Material.Ks", vec3(0.95f ,  0.95f, 0.95f));
    //prog.setUniform("Material.Ka", vec3(0.2f*0.3f, 0.55f* 0.3f, 0.9f* 0.3f));
    
  //  prog.setUniform("Material.Shininess", 100.0f);

//    model = mat4(1.0f);
 //   model = glm::translate(model,vec3(-1.0f, 0.75f, 3.0f));
 //   model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
  //  setMatrices();
  //  torus.render();

    //prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
  //  prog.setUniform("Material.Ks", vec3(0.9f ,0.9f,0.9f));
    //prog.setUniform("Material.Ks", vec3(0.2f, 0.2f, 0.2f));

//    prog.setUniform("Material.Shininess", 180.0f);

  //  model = mat4(1.0f);
    setMatrices();
    //plane.render();


}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}