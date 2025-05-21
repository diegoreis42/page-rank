#ifndef NODE_H
#define NODE_H
#include <string>
#include "utils/vec3d.h"
#include "utils/rgb.h"
#include <glm/glm.hpp>
using namespace std;

class Node
{
public:
    int id;

    Vec3D pos;
    Vec3D vel;
    Vec3D acc;
    double mass;
    float radius;
    double repulsion;
    double stiffness;
    double damping;
    int degree;
    Rgb color;

    Node(int _id);

    glm::vec3 get_glm_pos() const
    {
        return glm::vec3(pos.x, pos.y, pos.z);
    }
};
#endif