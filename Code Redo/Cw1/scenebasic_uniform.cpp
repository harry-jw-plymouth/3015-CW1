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



vec3 EyeCoordinates = vec3(1.0f, 1.25f, 1.25f);
vec3 CameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 CameraUp = vec3(0.0f, 1.0f, 0.0f);

vec3 Tree1Pos;

vec3 SwordPos;
vec3 SwordOffset = vec3(2.0f, 2.f, -15.0f);
vec3 SwordCenter;

enum VAO_IDs { Triangles, Indices, Colours, Textures, NumVAOs = 2 };
//VAOs
GLuint VAOs[NumVAOs];

//Buffer types
enum Buffer_IDs { ArrayBuffer, NumBuffers = 4 };
//Buffer objects
GLuint Buffers[NumBuffers];


#define RENDER_DISTANCE 256 //Render width of map
#define MAP_SIZE RENDER_DISTANCE * RENDER_DISTANCE //Size of map in x & z space
GLfloat(*terrainVertices)[6];
GLuint(*terrainIndices)[3];
const int squaresRow = RENDER_DISTANCE - 1;
const int trianglesPerSquare = 2;
const int trianglesGrid = squaresRow * squaresRow * trianglesPerSquare;
float TerrainSize;

mat4 mv;

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0.0f),
    angle(0.0f),
    rotSpeed(glm::pi<float>()/8.0f),
    sky(100.0f),
    plane(50.0f,50.0f,1,1),
    teapot(14,glm::mat4(1.0f)),
    torus(1.75f*0.75f,1.75f*0.75f,50,50) {
    ogre = ObjMesh::load("../Cw1/media/bs_ears.obj", false, true);
   /// mesh = ObjMesh::load("../Lab 1/media/pig_triangulated.obj",true);
}
void SceneBasic_Uniform::initScene()
{
    compile();
    SkyBoxProg.use();

    glEnable(GL_DEPTH_TEST);
    projection = mat4(1.0f);
    model = mat4(1.0f);
    angle = glm::radians(90.0f);
    GLuint CubeTex = Texture::loadHdrCubeMap("../Cw1/media/texture/cube/pisa-hdr/pisa");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, CubeTex);

    view = glm::lookAt(vec3(-1.0f, 0.25f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    
    prog.use();
   
    prog.setUniform("Spot.L", vec3(1.0f));
    prog.setUniform("Spot.La", vec3(0.05f));

    //GLuint texID = Texture::loadTexture("../Project_Template/media/texture/brick1.jpg");
    GLuint texID = Texture::loadTexture("media/texture/ogre_diffuse.png");
    GLuint texID2 = Texture::loadTexture("media/texture/ogre_normalmap.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);

    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texID2);

   
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

        SkyBoxProg.compileShader("shader/SkyBoxVertexShader.vert");
        SkyBoxProg.compileShader("shader/SkyBoxFragmentShader.frag");
        SkyBoxProg.link();
        SkyBoxProg.use();
    }
    catch (GLSLProgramException &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    float deltaT = t - tPrev;
    if (tPrev == 0.0f)deltaT = 0.0f;
    tPrev = t;
    angle += 0.25f * deltaT;
    if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
}
void SceneBasic_Uniform::setMatrices() {
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);

    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    prog.setUniform("MVP", projection * mv);

  //  SkyBoxProg.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    vec3 cameraPos = vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
    view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    //draw sky
    
    model = mat4(1.0f);
    


    SkyBoxProg.use();
    mat4 skyView = mat4(mat3(view));
    mat4 mv = skyView * mat4(1.0f);

    SkyBoxProg.setUniform("MVP", projection * mv);
    sky.render();


    prog.use();
    model = mat4(1.0f);
    setMatrices();
    vec4 lightPos = vec4(10.0f*cos(angle), 10.0f, 10.0f*sin(angle), 1.0f);
    prog.setUniform("Spot.Position", vec3(view * lightPos));
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    prog.setUniform("Spot.Direction", normalMatrix*vec3(-lightPos));

    prog.setUniform("Light.Position", view * vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));

    prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.09f));
    prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
    prog.setUniform("Material.Ka", vec3(0.2f*0.3f, 0.55f*0.3f, 0.9f*0.3f));  
    prog.setUniform("Material.Shininess", 100.0f);
   
    model = mat4(1.0f);
    setMatrices();
    ogre->render();

    setMatrices();


}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}