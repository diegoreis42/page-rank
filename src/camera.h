#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "utils/vec3d.h"

using namespace std;

class Camera
{
public:
    glm::vec3 pos;
    glm::vec3 target_pos;
    glm::vec3 angle;
    glm::mat4x4 view_mat;
    float viewport_width;
    float viewport_height;
    Camera(int vh, int vw);

    void update_view_mat(float dx, float dy);
    glm::mat4x4 get_view_mat();
    glm::vec3 right_vector();
    glm::vec3 view_dir();
    glm::vec3 up_vector();
    void set_viewport(int width, int height);
};
#endif