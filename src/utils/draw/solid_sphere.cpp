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
    // Vertex shader source code
    const char *vertexShaderSource = "#version 330 core \n"
                                     // Input vertex position at location 0
                                     "layout (location = 0) in vec3 aPos;\n"
                                     // Input vertex normal at location 1
                                     "layout (location = 1) in vec3 aNormal;\n"
                                     // Output to fragment shader: position in world space
                                     "out vec3 FragPos;\n"
                                     // Output to fragment shader: normal vector
                                     "out vec3 Normal;\n"
                                     // Model, view, and projection matrices as uniforms
                                     "uniform mat4 model;\n"
                                     "uniform mat4 view;\n"
                                     "uniform mat4 projection;\n"
                                     "void main()\n"
                                     "{\n"
                                     // Transform vertex position to world space
                                     "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
                                     // Pass normal directly (should be transformed for non-uniform scale)
                                     "   Normal = aNormal;\n"
                                     // Compute final vertex position in clip space
                                     "   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
                                     "}\0";

    // Fragment shader source code
    const char *fragmentShaderSource = "#version 330 core\n"
                                       // Output color
                                       "out vec4 FragColor;\n"
                                       // Input normal and position from vertex shader
                                       "in vec3 Normal;\n"
                                       "in vec3 FragPos;\n"
                                       // Uniforms for lighting
                                       "uniform vec3 lightDirection;\n"
                                       "uniform vec3 lightPos;\n"
                                       "uniform vec3 lightColor;\n"
                                       "uniform vec3 viewPos;\n"
                                       "uniform vec3 objectColor;\n"
                                       "void main() {\n"
                                       // Ambient lighting strength
                                       "   float ambientStrength = 0.15;\n"
                                       // Ambient component
                                       "   vec3 ambient = ambientStrength * lightColor;\n"
                                       // Normalize the light direction (note: should normalize normal instead)
                                       "   vec3 norm = normalize(lightDirection);\n"
                                       // Direction from fragment to light source
                                       "   vec3 lightDir = normalize(lightPos - FragPos);\n"
                                       // Diffuse component (Lambertian reflectance)
                                       "   float diff = max(dot(norm, lightDir), 0.0);\n"
                                       "   vec3 diffuse = diff * lightColor;\n"
                                       // Combine ambient and diffuse, modulate by object color
                                       "   float specularStrength = 1;"
                                       "   vec3 viewDir = normalize(viewPos - FragPos);"
                                       "   vec3 reflectDir = reflect(-lightDir, norm);"
                                       "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);"
                                       "   vec3 specular = specularStrength * spec * lightColor;  "
                                       "   vec3 result = (ambient + diffuse + specular) * objectColor;\n" // Output final color
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

void SolidSphere::setLightDirection(glm::vec3 dir)
{
    lightDirection = dir;
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
    glUniform3f(glGetUniformLocation(this->shaderProgram, "lightDirection"), this->lightDirection.x * -1, this->lightDirection.y * -1, this->lightDirection.z * -1);
    glUniform3fv(glGetUniformLocation(this->shaderProgram, "viewPos"), 1, glm::value_ptr(this->viewPos));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 3);
}