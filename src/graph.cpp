#include "graph.h"
#include <vector>
#include <unordered_set>
using namespace std;

Graph::Graph()
{
    this->adj_list.clear();
    this->node_list.clear();
}

int Graph::add_node()
{
    int new_node_id = adj_list.size();
    Node new_node(new_node_id);

    this->adj_list.push_back(unordered_set<int>());
    this->node_list.push_back(new_node);
    return new_node_id;
}

void Graph::add_edge(int node_id_1, int node_id_2)
{
    if (node_id_1 == node_id_2)
    {
        return;
    }
    this->adj_list[node_id_1].insert(node_id_2);
    this->adj_list[node_id_2].insert(node_id_1);
}

void Graph::update_degrees()
{
    for (int i = 0; i < this->adj_list.size(); i++)
    {
        this->node_list[i].degree = this->adj_list[i].size();
    }
}