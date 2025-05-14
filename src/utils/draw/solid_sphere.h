#ifndef SPHERE_H
#define SPHERE_H
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GL/gl.h>

class SolidSphere
{

protected:
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLushort> indices;
    int shaderProgram;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    unsigned int VBO, VAO;
    glm::vec3 color = glm::vec3(1, 1, 1);
public:
    glm::vec3 lightPos = glm::vec3(0, 0, 0);
    glm::vec3 lightDirection = glm::vec3(0, -0.5, 0);
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    
    SolidSphere(float radius, unsigned int rings, unsigned int sectors);
    void draw();
    void init();
    void setMVP(glm::mat4x4 m, glm::mat4x4 v, glm::mat4x4 p);
    void setColor(glm::vec3 c);
    void setLightColor(glm::vec3 c);
    void setLightPos(glm::vec3 pos);
    void setLightDirection(glm::vec3 dir);
};
#endif