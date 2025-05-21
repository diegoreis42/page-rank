#include "walker.h"

const float Walker::TRANSITION_DURATION = 3.0f;

Walker::Walker(int start_node) : current_node(start_node),
                                 transition_time(0.0f),
                                 transitioning(false),
                                 theta(0.0f),
                                 phi(0.0f) {}

void Walker::update(float dt)
{
    if (transitioning)
    {
        transition_time += dt;
        if (transition_time >= TRANSITION_DURATION)
        {
            transitioning = false;
            transition_time = 0.0f;
        }
    }
}

void Walker::move_to_node(int new_node)
{
    current_node = new_node;
    transitioning = true;
    transition_time = 0.0f;

    // Generate new random angles when moving to a new node
    theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    phi = static_cast<float>(rand()) / RAND_MAX * 3.14159f;
}
