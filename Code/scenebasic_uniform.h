#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/torus.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helper/plane.h"
#include "helper/objmesh.h"

class SceneBasic_Uniform : public Scene
{
private:
  //  GLuint vaoHandle;
    GLSLProgram prog;
  //  float angle;

    void compile();
    Torus torus;
   // Plane plane;
//    std::unique_ptr<ObjMesh>mesh;
public:
    SceneBasic_Uniform();

    void setMatrices();
    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
