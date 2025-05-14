#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "solid_sphere.h"

// Utility class for drawing a sphere
// Adapted to modern OpenGL from https://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c
SolidSphere::SolidSphere(float radius, unsigned int rings, unsigned int sectors)
{
    radius = radius;
}

void SolidSphere::init()
{
    const char *vertexShaderSource = "#version 330 core \n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "layout (location = 1) in vec3 aNormal;\n"
                                     "out vec3 FragPos;\n"
                                     "out vec3 Normal;\n"
                                     "uniform mat4 model;\n"
                                     "uniform mat4 view;\n"
                                     "uniform mat4 projection;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
                                     "   Normal = aNormal;\n"
                                     "   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
                                     "}\0";
    const char *fragmentShaderSource = "#version 330 core\n"
                                       "out vec4 FragColor;\n"
                                       "in vec3 Normal;\n"
                                       "in vec3 FragPos;\n"
                                       "uniform vec3 lightPos;\n"
                                       "uniform vec3 lightColor;\n"
                                       "uniform vec3 objectColor;\n"
                                       "void main() {\n"
                                       "   float ambientStrength = 0.1;\n"
                                       "   vec3 ambient = ambientStrength * lightColor;\n"
                                       "   vec3 norm = normalize(Normal);\n"
                                       "   vec3 lightDir = normalize(lightPos - FragPos);\n"
                                       "   float diff = max(dot(norm, lightDir), 0.0);\n"
                                       "   vec3 diffuse = diff * lightColor;\n"
                                       "   vec3 result = (ambient + diffuse) * objectColor;\n"
                                       "   FragColor = vec4(result, 1.0);\n"
                                       "}\0";

    int success;
    char infoLog[512];

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    // check for shader compile errors
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    // check for shader compile errors
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLfloat x, y, z, alpha, beta; // Storage for coordinates and angles
    GLfloat radius = 1.0f;
    int gradation = 20;
    vertices.clear();
    for (alpha = 0.0; alpha < M_PI; alpha += M_PI / gradation)
    {
        for (beta = 0.0; beta < 2.01 * M_PI; beta += M_PI / gradation)
        {
            x = radius * cos(beta) * sin(alpha);
            y = radius * sin(beta) * sin(alpha);
            z = radius * cos(alpha);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            x = radius * cos(beta) * sin(alpha + M_PI / gradation);
            y = radius * sin(beta) * sin(alpha + M_PI / gradation);
            z = radius * cos(alpha + M_PI / gradation);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,        // attribute. No particular reason for 1, but must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void SolidSphere::setMVP(glm::mat4 m, glm::mat4 v, glm::mat4 p)
{
    model = m;
    view = v;
    projection = p;
}

void SolidSphere::setColor(glm::vec3 c)
{
    color = c;
}

void SolidSphere::setLightColor(glm::vec3 c)
{
    lightColor = c;
}

void SolidSphere::setLightPos(glm::vec3 pos)
{
    lightPos = pos;
}

glm::vec3 SolidSphere::getLightColor()
{
    return this->lightColor;
}

glm::vec3 SolidSphere::getLightPos()
{
    return this->lightPos;
}

void SolidSphere::draw()
{
    glUseProgram(this->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(this->shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(this->model));
    glUniformMatrix4fv(glGetUniformLocation(this->shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(this->view));
    glUniformMatrix4fv(glGetUniformLocation(this->shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection));
    glUniform3fv(glGetUniformLocation(this->shaderProgram, "objectColor"), 1, glm::value_ptr(this->color));
    glUniform3fv(glGetUniformLocation(this->shaderProgram, "lightColor"), 1, glm::value_ptr(this->lightColor));
    glUniform3fv(glGetUniformLocation(this->shaderProgram, "lightPos"), 1, glm::value_ptr(this->lightPos));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 3);
}