#include "graph.h"
#include "walker.h"
#include <random>

class Universe
{
public:
    float dt;
    float repulsion;
    float spring_k;
    float damping;
    float gravity;
    int n_iterations;
    Graph graph;
    std::vector<Walker> walkers;
    static const int NUM_WALKERS = 1000;

    Universe(Graph graph,
             float dt,
             float repulsion,
             float spring_k,
             float damping,
             float gravity);

    void update(float deltaT);
    Vec3D compute_spring_force(Node n1, Node n2);
    Vec3D compute_repulsion_force(Node n1, Node n2);
    Vec3D compute_spring_force_general(float k, Vec3D v1, Vec3D v2);
    void set_graph(Graph graph);
    void init_walkers();
    void update_walkers(float deltaT);
};