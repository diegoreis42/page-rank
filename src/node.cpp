#include "node.h"
#include "utils/rgb.h"
#include <string>

using namespace std;

Node::Node(int id) : id{id},
                     pos{Vec3D::random(1.0)},
                     vel{Vec3D::random(1.0)},
                     acc{Vec3D::random(1.0)},
                     color{Rgb::random()},
                     mass{1.0},
                     radius{1.0},
                     repulsion{1.0},
                     stiffness{1.0},
                     damping{1.0},
                     degree{0}
{
}