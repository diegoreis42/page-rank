#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <unordered_set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include "camera.h"
#include "universe.h"
#include "graph.h"
#include "utils/draw/solid_sphere.h"
#include "utils/draw/line.h"

typedef int32_t i32;
typedef uint32_t u32;
typedef int32_t b32;

#define WIN_WIDTH 1024
#define WIN_HEIGHT 1024

using namespace std;
bool movingUp = false;
float yLocation = 0.0f;
long int n_iterations = INT_FAST32_MAX;

Graph graph;
Camera camera(WIN_WIDTH, WIN_HEIGHT);

double mouseX = 0;
double mouseY = 0;
double mouseScroll = 0;
float scroll_sensitivity = 1.0f;
float drag_sensitivity = 1.5;
float zoom = 0.0f;
bool mouseDown = false;
bool mouseDownFirst = false;
bool autoRotateX = true;
bool show_degree = true;
float MIN_DISTANCE_ORIGIN = 5.0f;
float zoom_step = 2.5f;
glm::mat4 projection;
glm::mat4 view;

int n_random_nodes = 100;
float timeDelta = 0.2;
float repulsion_force = 1.0f;
float spring_force = 1.0f;
float damping_coefficient = 0.5f;
float gravitational_force = 0.5f;

Universe universe(graph,
                  timeDelta,
                  repulsion_force,
                  spring_force,
                  damping_coefficient,
                  gravitational_force);
SolidSphere sphere(
    10.0f, 12, 24);
Line line(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));

// The graph is initialized with random nodes and edges
// using a preferential attachment mechanism to mimic a scale-free graph
// That is the structure of the world wide web: https://www.networksciencebook.com/chapter/4#scale-free
void init_graph()
{
    graph.adj_list.clear();
    graph.node_list.clear();

    std::srand(time(NULL));

    for (int i = 0; i < n_random_nodes; i++)
    {
        int n3 = graph.add_node("C");

        // Use a preferential attachment mechanism to mimic a scale-free graph
        int total_degree = 0;
        for (const auto &node : graph.node_list)
        {
            total_degree += node.degree;
        }

        if (total_degree == 0)
        {
            // If no edges exist yet, connect to a random node
            int rd = rand() % (graph.node_list.size());
            graph.add_edge(n3, rd);
        }
        else
        {
            // Preferentially attach based on node degree
            for (int j = 0; j < graph.node_list.size(); j++)
            {
                float probability = (float)graph.node_list[j].degree / total_degree;
                if ((float)rand() / RAND_MAX < probability)
                {
                    graph.add_edge(n3, j);
                }
            }
        }
    }

    // Ensure every node is reachable from any other node
    for (int i = 0; i < graph.node_list.size(); i++)
    {
        std::unordered_set<int> visited;
        std::function<void(int)> dfs = [&](int node)
        {
            visited.insert(node);
            for (int neighbor : graph.adj_list[node])
            {
                if (visited.find(neighbor) == visited.end())
                {
                    dfs(neighbor);
                }
            }
        };

        dfs(i);

        for (int j = 0; j < graph.node_list.size(); j++)
        {
            if (visited.find(j) == visited.end())
            {
                graph.add_edge(i, j);
            }
        }
    }

    universe.set_graph(graph);
}

void draw_graph(float yloc)
{
    static int n_nodes = universe.graph.adj_list.size();

    for (int i = 0; i < universe.graph.adj_list.size(); i++)
    {
        Node node_i = universe.graph.node_list[i];
        unordered_set<int> neighbors = universe.graph.adj_list[i];
        for (auto j = neighbors.begin(); j != neighbors.end(); ++j)
        {
            Node node_j = universe.graph.node_list[*j];
            line.setVertices(
                glm::vec3(node_i.pos.x, node_i.pos.y + yloc, node_i.pos.z),
                glm::vec3(node_j.pos.x, node_j.pos.y + yloc, node_j.pos.z));
            line.setMVP(projection * view);
            line.draw();
        }
    }

    // Find max degree for normalization
    int max_degree = 1;
    for (const auto &node : universe.graph.node_list)
    {
        max_degree = std::max(max_degree, node.degree);
    }

    for (int i = 0; i < n_nodes; i++)
    {
        Node nd = universe.graph.node_list[i];

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(nd.pos.x, nd.pos.y + yloc, nd.pos.z));

        // Calculate radius based on node degree
        float min_radius = 0.5f;
        float max_radius = 2.5f;
        float radius = min_radius + (max_radius - min_radius) * (float)nd.degree / max_degree;

        glm::vec3 scale = glm::vec3(radius, radius, radius);
        model = glm::scale(model, scale);
        sphere.setColor(nd.color.color);
        sphere.setMVP(model, view, projection);
        sphere.draw();
    }
}

void render(void)
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view = camera.get_view_mat();

    draw_graph(yLocation);
}

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_W)
    {
        camera.update_view_mat(0.1, 0);
    }
    if (key == GLFW_KEY_S)
    {
        camera.update_view_mat(-0.1, 0);
    }
    if (key == GLFW_KEY_A)
    {
        camera.update_view_mat(0, 0.1);
    }
    if (key == GLFW_KEY_D)
    {
        camera.update_view_mat(0, -0.1);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        autoRotateX = !autoRotateX;
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{

    if (mouseDownFirst)
    {
        cout << "Mouse clicked" << endl;
        mouseX = xpos;
        mouseY = ypos;
        mouseDownFirst = false;
    }

    if (mouseDown)
    {
        double dx = (xpos - mouseX) * drag_sensitivity;
        double dy = (ypos - mouseY) * drag_sensitivity;
        mouseX = xpos;
        mouseY = ypos;
        camera.update_view_mat(dx, dy);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    cout << "Scroll clicked" << endl;
    if (glm::distance(camera.pos, glm::vec3(0.0f)) > MIN_DISTANCE_ORIGIN)
    {
        camera.pos += (float)yoffset * scroll_sensitivity * camera.view_dir();
    }
    else
    {
        camera.pos -= camera.view_dir() * scroll_sensitivity;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.set_viewport(width, height);
}

std::function<void()> loop;
void main_loop() { loop(); }

int main()
{
    cout << "### Starting Page Rank ###" << endl;
    srand(time(NULL));

    cout << "Initializing graph..." << endl;
    init_graph();

    GLFWwindow *window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return EXIT_FAILURE;

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Graph Viewer", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    projection = glm::perspective(
        glm::radians<float>(60.0f), (GLfloat)WIN_WIDTH / (GLfloat)WIN_HEIGHT,
        0.1f, 500.0f);

    sphere.init();
    line.init();

    glEnable(GL_DEPTH_TEST);

    b32 running = true;
    b32 fullscreen = false;

    double timeAccumulator = 0;
    double timeSimulatedThisIteration = 0;
    double currentTime = glfwGetTime();
    double startTime = glfwGetTime();
    double initTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        timeSimulatedThisIteration = 0;
        startTime = glfwGetTime();

        if (universe.n_iterations < n_iterations)
        {
            universe.update(timeDelta);
        }

        currentTime = glfwGetTime();

        if (autoRotateX)
        {
            camera.update_view_mat(2.5, 0);
        }

        render();

        currentTime = glfwGetTime();
        timeAccumulator += currentTime - startTime;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}