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

vec3 SwordPos;
vec3 SwordOffset=vec3(2.0f,2.f,-15.0f);
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

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0),
    angle(0.0f),
    SkyBox(100.0f),
    plane(50.0f,50.0f,1,1),
    teapot(14,glm::mat4(1.0f)),
    torus(1.75f*0.75f,1.75f*0.75f,50,50) {
    SwordInStone = ObjMesh::load("../Cw1/media/low poly sword in stone.obj",true);
   /// mesh = ObjMesh::load("../Lab 1/media/pig_triangulated.obj",true);
}
void SceneBasic_Uniform::SetUpTerrain() {
    terrainVertices= new GLfloat[MAP_SIZE][6];
    terrainIndices = new GLuint[trianglesGrid][3];
   // float SpaceBetween = 0.0625f;
    float SpaceBetween = 0.1f;

    TerrainSize = (RENDER_DISTANCE - 1) * SpaceBetween;

    //Positions to start drawing from (centered around origin)
    float drawingStartPosition = 10.0f;
    //float columnVerticesOffset = (EyeCoordinates + CameraFront).x + (((RENDER_DISTANCE - 1) * SpaceBetween) / 2);
    //float rowVerticesOffset= (EyeCoordinates + CameraFront).z + (((RENDER_DISTANCE - 1) * SpaceBetween) / 2);
    float columnVerticesOffset = drawingStartPosition;
    float rowVerticesOffset = drawingStartPosition;

    int rowIndex = 0;
    for (int i = 0; i < MAP_SIZE; i++)
    {
        //Generation of x & z vertices for horizontal plane
        terrainVertices[i][0] = columnVerticesOffset;
        terrainVertices[i][1] = 0.25f;
        terrainVertices[i][2] = rowVerticesOffset;

        //Colour
        terrainVertices[i][3] = 0.0f;
        terrainVertices[i][4] = 0.75f;
        terrainVertices[i][5] = 0.25f;

        //Shifts x position across for next triangle along grid
        columnVerticesOffset = columnVerticesOffset + -SpaceBetween;

        //Indexing of each chunk within row
        rowIndex++;
        //True when all triangles of the current row have been generated
        if (rowIndex == RENDER_DISTANCE)
        {
            //Resets for next row of triangles
            rowIndex = 0;
            //Resets x position for next row of triangles
            columnVerticesOffset = drawingStartPosition;
            //Shifts z position
            rowVerticesOffset = rowVerticesOffset + -SpaceBetween;
        }
    }
    

    //Positions to start mapping indices from
    int columnIndicesOffset = 0;
    int rowIndicesOffset = 0;

    //Generation of map indices in the form of chunks (1x1 right angle triangle squares)
    rowIndex = 0;
    for (int i = 0; i < trianglesGrid - 1; i += 2)
    {
        terrainIndices[i][0] = columnIndicesOffset + rowIndicesOffset; //top left
        terrainIndices[i][2] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
        terrainIndices[i][1] = 1 + columnIndicesOffset + rowIndicesOffset; //top right

        terrainIndices[i + 1][0] = 1 + columnIndicesOffset + rowIndicesOffset; //top right
        terrainIndices[i + 1][2] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
        terrainIndices[i + 1][1] = 1 + RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom right

        //Shifts x position across for next chunk along grid
        columnIndicesOffset = columnIndicesOffset + 1;

        //Indexing of each chunk within row
        rowIndex++;

        //True when all chunks of the current row have been generated
        if (rowIndex == squaresRow)
        {
            //Resets for next row of chunks
            rowIndex = 0;
            //Resets x position for next row of chunks
            columnIndicesOffset = 0;
            //Shifts z position
            rowIndicesOffset = rowIndicesOffset + RENDER_DISTANCE;
        }
    }
    //Sets index of VAO
    glGenVertexArrays(NumVAOs, VAOs);
    //Binds VAO to a buffer
    glBindVertexArray(VAOs[0]);
    //Sets indexes of all required buffer objects
    glGenBuffers(NumBuffers, Buffers);

    //Binds vertex object to array buffer
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[Triangles]);
    //Allocates buffer memory for the vertices of the 'Triangles' buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_SIZE * 6, terrainVertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

    //Binding & allocation for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Indices]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), terrainIndices, GL_STATIC_DRAW);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * trianglesGrid * 3, terrainIndices, GL_STATIC_DRAW);

    //Allocation & indexing of vertex attribute memory for vertex shader
    //Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbinding
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void SceneBasic_Uniform::initScene()
{
    compile();
    SkyBoxShaders.use();

    glEnable(GL_DEPTH_TEST);

    projection = mat4(1.0f);
    model = mat4(1.0f);
    angle = glm::radians(90.0f);
    GLuint SkyBoxTexture = Texture::loadCubeMap("../Cw1/media/texture/cube/pisa/Forest");
  //  GLuint SkyBoxTexture = Texture::loadHdrCubeMap("../Cw1/media/texture/Skybox/Forest/forest-skyboxes/Brudslojan");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBoxTexture);

    
    view = glm::lookAt((EyeCoordinates), CameraFront, CameraUp);

    prog.use();
    prog.setUniform("Tex1", 0);

    projection = mat4(1.0f);
    angle = 0.0f;

    prog.use();
    prog.setUniform("Tex1", 0);
    prog.setUniform("Spot.L", vec3(0.99f));//light intensity
    prog.setUniform("Spot.La", vec3(0.5f)); //light intensity outside spotlight

    //GLuint texID = Texture::loadTexture("../Project_Template/media/texture/brick1.jpg");
   // GLuint texID = Texture::loadTexture("media/texture/brick1.jpg");
   // GLuint texID = 
    SwordTexture = Texture::loadTexture("media/texture/SwordTexture.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SwordTexture);

   
    prog.setUniform("Spot.La", vec3(0.5f));  //light intensity outside spotlight
    prog.setUniform("Spot.Exponent", 8.0f);  // higher value =darker around edges of light
    prog.setUniform("Spot.Cutoff", glm::radians(40.0f)); //size of light (higher == bigger spotlight area) 

   

    model = glm::translate(model, EyeCoordinates + CameraFront * 2.0f);
    model = glm::scale(model, vec3(5.0f));
    model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));

    SwordPos = EyeCoordinates + CameraFront;

    SetUpTerrain();
    TerrainShaders.use();
    TerrainShaders.setUniform("TerrainStart", glm::vec2(10.0f, 10.0f));
    TerrainShaders.setUniform("TerrainSize", TerrainSize);
    // TerrainShaders.setUniform("mvpIn", projection * mv);

    GroundTexture = Texture::loadTexture("media/texture/SkyBoxBottom.png");
    // GroundTexture = Texture::loadTexture("media/texture/Ground.png");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GroundTexture);
    TerrainShaders.setUniform("GroundTexture", 0);
    


}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/NewVertexShader.vert");
        prog.compileShader("shader/NewFragmentShader.frag");
        prog.link();
        prog.use();

        SkyBoxShaders.compileShader("shader/SkyBoxVertexShader.vert");
        SkyBoxShaders.compileShader("shader/SkyBoxFragmentShader.frag");
        SkyBoxShaders.link();
      //  SkyBoxShaders.use();

        TerrainShaders.compileShader("shader/Terrainfragmentshader.frag");
        TerrainShaders.compileShader("shader/TerrainVertexshader.vert");
        TerrainShaders.link();


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
    angle += 0.125f * deltaTime;
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
    const float sensitivity = 0.05f;
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


    //draw terrain
    TerrainShaders.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GroundTexture);
    TerrainShaders.setUniform("GroundTexture", 0);

    mat4 mv = view * glm::mat4(1.0f);
    TerrainShaders.setUniform("mvpIn", projection * mv);
    TerrainShaders.setUniform("TerrainStart", glm::vec2(10.0f, 10.0f));
    TerrainShaders.setUniform("TerrainSize", TerrainSize);
    // TerrainShaders.setUniform("mvpIn", projection * mv);

    glBindVertexArray(VAOs[0]);
    glDrawElements(GL_TRIANGLES, trianglesGrid * 3, GL_UNSIGNED_INT, 0);

 


    //draw sky
    model = mat4(1.0f);

    glDepthMask(GL_FALSE);          // disable depth writes
    glDepthFunc(GL_LEQUAL);

    SkyBoxShaders.use();
    mat4 skyView = mat4(mat3(view));
    mv = skyView * mat4(1.0f);

    SkyBoxShaders.setUniform("MVP", projection * mv);
    SkyBox.render();

    glDepthMask(GL_TRUE);      
    glDepthFunc(GL_LESS);           
    //draw cube

    prog.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SwordTexture);
  //  prog .setUniform("Tex1", 0);
    prog.setUniform("Tex1", 0);
    model = mat4(1.0f);
    setMatrices();

    vec4 lightPos = vec4(10.0f*cos(angle), 10.0f, 10.0f*sin(angle), 1.0f);
    prog.setUniform("Spot.Position", vec3(view * lightPos));
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));

//    vec3 swordPosView = vec3(view * vec4(SwordPos, 1.0f));
 //   vec3 spotDirView = normalize(swordPosView - lightPosView);

   // prog.setUniform("Spot.Direction", spotDirView);

    prog.setUniform("Spot.Direction", normalize(vec3(view*vec4(SwordPos,1.0f))-vec3(view* lightPos)));

    prog.setUniform("Material.Kd", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ka", vec3(0.2f*0.3f, 0.55f*0.3f, 0.9f*0.3f));  
    prog.setUniform("Material.Shininess", 100.0f);

    model = mat4(1.0f);
    model = glm::translate(model, SwordPos);
    
    model = glm::translate(model, vec3(-0.0, -2.0f, 10.0f)+SwordOffset);
  //  model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
  //  model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    
   
   // model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
   // SwordInStone->render();
    SwordInStone->render();

   // prog.setUniform("Material.Kd", vec3a(0.2f, 0.55f, 0.9f));
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