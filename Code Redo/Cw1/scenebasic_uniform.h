#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helper/plane.h"
#include "helper/cube.h"
#include "helper/objmesh.h"
#include "helper/texture.h"
#include "helper/skybox.h"

class SceneBasic_Uniform : public Scene
{
private:
  //  GLuint vaoHandle;
    GLSLProgram prog;

    GLSLProgram SkyBoxShaders;
    GLSLProgram TerrainShaders;
  //  float angle;

    Cube cube;
    SkyBox SkyBox;
    GLuint SwordTexture;
    GLuint GroundTexture;

    std::unique_ptr<ObjMesh>SwordInStone;
    std::unique_ptr<ObjMesh>Tree;

    float angle;
    float tPrev;
    float rotSpeed;
    void compile();
    Torus torus;
    Teapot teapot;
    Plane plane;
    std::unique_ptr<ObjMesh>ogre;
 //   std::unique_ptr<ObjMesh>mesh;
public:
    SceneBasic_Uniform();
    void DrawSword();
    void DrawTrees();
    void DrawSkyBox();
    void DrawTerrain();
    void SetUpTerrain();
    void setMatrices();
    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void ProcessUserInput(int key, int action)override;
    void Mouse_CallBack(double xpos, double ypos)override;
};

#endif // SCENEBASIC_UNIFORM_H
