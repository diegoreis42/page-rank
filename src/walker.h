#ifndef WALKER_H
#define WALKER_H

#include <glm/glm.hpp>

class Walker
{
public:
    int current_node;
    float transition_time;
    bool transitioning;
    float theta; // Stored angle around node
    float phi;   // Second angle for positioning

    Walker(int start_node);
    void update(float dt);
    void move_to_node(int new_node);
    static const float TRANSITION_DURATION;
};

#endif
