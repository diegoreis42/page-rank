#ifndef RGB_H
#define RGB_H
#include <cstdint>
#include "../../deps/glm/glm/glm.hpp"
#include "../../deps/glm/glm/vec3.hpp"

class Rgb
{
public:
    glm::vec3 color;
    static Rgb random();
    Rgb(float r, float g, float b);
};
#endif