#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include "GLFW/glfw3.h";

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

enum VAO_IDs { Triangles, Indices, Colours, Textures, NumVAOs,TerrainVAO=0 };
//VAOs
GLuint VAOs[NumVAOs];

//Buffer types
enum Buffer_IDs { TerrainVertexBuffer=0,TerrainIndexBuffer,ArrayBuffer, NumBuffers  };
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
    SkyBox(100.0f){
    Butterfly = ObjMesh::load("../Cw1/media/Butterfly/_butterfly.obj");
    SwordInStone = ObjMesh::load("../Cw1/media/low poly sword in stone.obj", true);
    Tree = ObjMesh::load("../Cw1/media/Tree.obj");
}
void SceneBasic_Uniform::SetUpTerrain() {
    //Code re used with some adaptions from 3016

    terrainVertices = new GLfloat[MAP_SIZE][6];
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
    glBindVertexArray(VAOs[TerrainVAO]);
    //Sets indexes of all required buffer objects
    glGenBuffers(NumBuffers, Buffers);

    //Binds vertex object to array buffer
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[TerrainVertexBuffer]);
    //Allocates buffer memory for the vertices of the 'Triangles' buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_SIZE * 6, terrainVertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

    //Binding & allocation for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[TerrainIndexBuffer]);
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
   // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
}
void SceneBasic_Uniform::SetupSkybox() {
    SkyBoxShaders.use();

    glEnable(GL_DEPTH_TEST);

    projection = mat4(1.0f);
    model = mat4(1.0f);
    angle = glm::radians(90.0f);
    GLuint SkyBoxTexture = Texture::loadCubeMap("../Cw1/media/texture/cube/pisa/Forest");
    //  GLuint SkyBoxTexture = Texture::loadHdrCubeMap("../Cw1/media/texture/Skybox/Forest/forest-skyboxes/Brudslojan");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBoxTexture);
}
void SceneBasic_Uniform::LoadTextures() {
    SwordTexture = Texture::loadTexture("media/texture/SwordTexture.png");
    MossTexture = Texture::loadTexture("media/texture/moss.png");
    ButterflyTexture = Texture::loadTexture("media/Butterfly/texture.bmp");
    TreeTexture= Texture::loadTexture("media/texture/TreeTexture.png");
    GroundTexture = Texture::loadTexture("media/texture/SkyBoxBottom.png");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SwordTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, MossTexture);
}
void SceneBasic_Uniform::LoadTerrain() {
    //set up terrain
    TerrainShaders.use();
    TerrainShaders.setUniform("TerrainStart", glm::vec2(10.0f, 10.0f));
    TerrainShaders.setUniform("TerrainSize", TerrainSize);

    SetUpTerrain();
   
    //bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GroundTexture);
    TerrainShaders.setUniform("GroundTexture", 0);
}
void SceneBasic_Uniform::SetupButterflyStart() {
    //Set start for for butterfly
    MainButterflyPos = vec3(1.0f, 2.3f, 1.0f);
    MainButterflyRotaion = vec3(0.0f, 90.0f, 0.0f);
    MainButterflyScale = vec3(0.003f);

    ButterflyModel = glm::rotate(ButterflyModel, glm::radians(-45.0f), vec3(0.0f, 1.0f, 0.0f));
}
float SceneBasic_Uniform::gauss(float x, float sigma2) {
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}
void SceneBasic_Uniform::SetupFBO() {
    std::cout << "SetupFBO: " << width << "x" << height << std::endl;
    
    // generate and bind the freamework
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    //create the texture object
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    //bind tectures to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
    //Create the depth buffer 

    GLuint DepthBuf;
    glGenRenderbuffers(1, &DepthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, DepthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    //Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuf);
    //Set the targets fro the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    //Unbind the framebuffer and revert to the default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    glGenTextures(1, &intermediateTex);
    glActiveTexture(GL_TEXTURE1);        // use slot 1 instead of 0
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTex, 0);
    glDrawBuffers(1, drawBuffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //array for full screen quad
    GLfloat verts[] = {
        -1.0f,-1.0f,0.0f,1.0f,-1.0f,0.0f,1.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f
    };
    GLfloat tc[] = {
        0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,
        0.0f,0.0f,1.0f,1.0f,0.0f,1.0f
    };
    //buffer set up
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

    // set up vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);//vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);// texture coordinates
    glBindVertexArray(0);

    prog.use();
    prog.setUniform("Tex1", 0);
    prog.setUniform("MossTex", 1);

    float weights[5], sum, sigma2 = 8.0f;
    weights[0] = gauss(0, sigma2);
    sum = weights[0];
    for (int i = 1; i < 5; i++) {
        weights[i] = gauss(float(i), sigma2);
        sum += 2 * weights[i];
    }
    for (int i = 0; i < 5; i++) {
        std::stringstream uniName;
        uniName << "Weight[" << i << "]";
        float val = weights[i] / sum;
        prog.setUniform(uniName.str().c_str(), val);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "FBO incomplete\n";

    prog.use();
    prog.setUniform("TexelSize", glm::vec2(1.0f / width, 1.0f / height));


}
void SceneBasic_Uniform::initScene()
{
    compile();

    

    SetupSkybox();

    //Set camera start 
    view = glm::lookAt((EyeCoordinates), CameraFront, CameraUp);    

    //Get object textures
    prog.use();
    LoadTextures();  

    // Set light values 
    prog.setUniform("Spot.L", vec3(1.0f));
    prog.setUniform("Spot.La", vec3(0.5f));
    prog.setUniform("Spot.Exponent", 50.0f);
    prog.setUniform("Spot.Cutoff", glm::radians(15.0f));

    LoadTerrain();

    SetupButterflyStart();


    SetupFBO();
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "renderFBO incomplete: " << status << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "intermediateFBO incomplete: " << status << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
void SceneBasic_Uniform::ProcessUserInput(int key, int action) {
    const float movementSpeed = 2.0f * deltaTime;

    if (action == GLFW_PRESS) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_W) {
                EyeCoordinates += movementSpeed * CameraFront;
                 //  std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_A) {
                EyeCoordinates -= normalize(cross(CameraFront, CameraUp)) * movementSpeed;
               //  std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_S) {
                EyeCoordinates -= movementSpeed * CameraFront;
               //   std::cout << "Key: " << key << std::endl;
            }
            else if (key == GLFW_KEY_D) {
                EyeCoordinates += normalize(cross(CameraFront, CameraUp)) * movementSpeed;
                
                
                //std::cout << "Key: " << key << std::endl;
            }
        }
    }
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/FinalVertexShader.vert");
        prog.compileShader("shader/FinalFragmentShader.frag");
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
    float deltaT = t - tPrev;
    if (tPrev == 0.0f)deltaT = 0.0f;
    tPrev = t;
    angle += 0.05f * deltaT;
    if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
    deltaTime = deltaT;

    //update view for updated eye coorindates
    view = glm::lookAt(EyeCoordinates, EyeCoordinates + CameraFront, CameraUp);

    //Move pos of butterfly for scene animation
    MoveButterflies();
}
void SceneBasic_Uniform::setMatrices() {
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);

    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    prog.setUniform("MVP", projection * mv);

  //  SkyBoxProg.setUniform("MVP", projection * mv);
}
void SceneBasic_Uniform::DrawTerrain() {
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

    glBindVertexArray(VAOs[TerrainVAO]);
    glDrawElements(GL_TRIANGLES, trianglesGrid * 3, GL_UNSIGNED_INT, 0);

}
void SceneBasic_Uniform::DrawSkyBox() {
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

    prog.use();
}
void SceneBasic_Uniform::DrawSword() {
    prog.use();;

    prog.setUniform("Pass", 1); // FIX: re-set pass after terrain/skybox shader switches

    vec4 lightPos = vec4(10.0f * cos(angle), 10.0f, 10.0f * sin(angle), 1.0f);
    prog.setUniform("Spot.Position", vec3(view * lightPos));
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    prog.setUniform("Spot.Direction", normalMatrix * vec3(-lightPos));
    prog.setUniform("Light.Position", view * vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));
    prog.setUniform("Material.Kd", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f));
    prog.setUniform("Material.Shininess", 100.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SwordTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, MossTexture);
    prog.setUniform("Tex1", 0);
    prog.setUniform("MossTex", 1);

    model = mat4(1.0f);
    SwordPos = vec3(0.0f, 1.3f, 0.0f);
    model = glm::translate(model, SwordPos);
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    SwordInStone->render();
}
void SceneBasic_Uniform::DrawTree(vec3 Pos, vec3 Rotation,vec3 Scale) {

   
    model = mat4(1.0f);
    model = glm::translate(model, Pos);
    model = glm::scale(model, Scale);
    model = glm::rotate(model, glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f));
    setMatrices();
    Tree->render();
}
void SceneBasic_Uniform::DrawTrees() {
    prog.use();
    prog.setUniform("Pass", 1); // FIX
    prog.setUniform("Tex1", 0);
    prog.setUniform("MossTex", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TreeTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, MossTexture);

    prog.setUniform("Material.Kd", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ka", vec3(0.5f));
    prog.setUniform("Material.Shininess", 100.0f);

     DrawTree(vec3(0.0f, 0.2f, -2.5f), vec3(0.0f, -14.0f, -5.0f), vec3(0.8f));
     DrawTree(vec3(-4.0f, 0.2f, -2.0f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-5.0f, 0.2f, 1.0f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(4.0f, 0.2f, 2.0f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(5.0f, 0.2f, -2.3f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-1.0f, 0.2f, -5.0f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(5.5f, 0.2f, 5.0f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-5.5f, 0.2f, 5.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-3.5f, 0.2f, -6.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(3.2f, 0.2f, -5.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(0.0f, 0.2f, -6.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-7.0f, 0.2f, 0.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-9.0f, 0.2f, 0.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(-9.0f, 0.2f, 9.4f), vec3(0.0f), vec3(0.9f));
     DrawTree(vec3(9.0f, 0.2f, 9.4f), vec3(0.0f), vec3(0.9f));
}
int ButterflyYDirCount = 10;
int ButterflyXDirCount = 0;
int RotateFrame = 16;
float Rotation = 45.0f;
bool CurrentlyMovingUp = true; bool CurrentlyMovingForward = false;
void SceneBasic_Uniform::MoveButterflies() {
    float YSpeed = 0.1f;
    float XSpeed = 0.2f;
    float ZSpeed = 0.2f;
    ButterflyYDirCount++;
    ButterflyXDirCount++;

    if (ButterflyYDirCount == 200) {
        CurrentlyMovingUp = !CurrentlyMovingUp;
        ButterflyYDirCount = 0;
    }
    if (ButterflyXDirCount == 6000) {
        ButterflyXDirCount = 0;
        CurrentlyMovingForward = !CurrentlyMovingForward;
        RotateFrame = 0;
    }
    if (!CurrentlyMovingUp) {
        YSpeed = -0.1f;
    }
    if (!CurrentlyMovingForward) {
        XSpeed = -0.2f;
        ZSpeed = -0.2f;
    }
    if (RotateFrame < 16) {
        Rotation += 11.25f;
        RotateFrame++;
        //cout<<"Rotate frame \n";
    }
    MainButterflyPos.x += XSpeed * deltaTime;
    MainButterflyPos .y += YSpeed * deltaTime;
    MainButterflyPos.z += XSpeed * deltaTime;
  //  MainButterflyRotaion.y += Rotation;
    ButterflyModel = mat4(1.0f);
    ButterflyModel = translate(ButterflyModel, MainButterflyPos);
    ButterflyModel = rotate(ButterflyModel, radians(Rotation), vec3(0.0f, 1.0f, 0.0f));
    ButterflyModel = scale(ButterflyModel, MainButterflyScale);
    
}

void SceneBasic_Uniform::DrawButterfly(glm::vec3 Pos, mat4 ButterFlyModel) {
    model = ButterFlyModel;
    setMatrices();
    Butterfly->render();

}
void SceneBasic_Uniform::DrawButterflies() {
    prog.use();
    prog.setUniform("Pass", 1); // FIX
    prog.setUniform("Tex1", 0);
    prog.setUniform("MossTex", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ButterflyTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ButterflyTexture);

    DrawButterfly(MainButterflyPos, ButterflyModel);

}
void SceneBasic_Uniform::Pass1() {
    prog.use();
    prog.setUniform("Pass", 1);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                              // FIX 1: explicit clear colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawSkyBox();
    prog.use();                                                          // FIX 3: reclaim prog after skybox
    prog.setUniform("Pass", 1);

    DrawTerrain();

    prog.use();                                                          // FIX 5: reclaim prog after terrain shader
    prog.setUniform("Pass", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SwordTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, MossTexture);
    prog.setUniform("Tex1", 0);                                         // FIX 7: ensure Tex1 points to slot 0
    prog.setUniform("MossTex", 1);
    DrawSword();

    DrawTrees();

    DrawButterflies();

}
void SceneBasic_Uniform::Pass2() {
    prog.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::Pass3() {
    prog.setUniform("Pass", 3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void SceneBasic_Uniform::render()
{
    prog.use();
    Pass1();
    Pass2();
    Pass3();

}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

   
}