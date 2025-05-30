#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <unordered_set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h> // For getting the executable path
#endif

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

double mouseScroll = 0;
float scroll_sensitivity = 1.0f;
float zoom = 0.0f;
bool mouseDown = false;
bool mouseDownFirst = false;
bool autoRotateX = false;
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

float walker_update_interval = 2.0f;
float walker_timer = 0.0f;

const int LAST_STEP = 100; // only for testing, probably this won't be known at compile-time
struct pagerank_step_context
{
    int current_step = 0;
    int total_steps = LAST_STEP;

    // could be a function that lazy-loads the description instead
} step;

struct imgui_context
{
    bool showing_about = false;
    bool last_frame_showing_about = false;
    ImVec2 steps_window_start_pos;
    bool showing_steps_window = true;
    float light_pitch, light_yaw, z_rad;
    bool light_at_camera = false;
} imgui_context;

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
        int n3 = graph.add_node();
        int rd = rand() % (graph.node_list.size());
        graph.add_edge(n3, rd);

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

    if (imgui_context.light_at_camera)
        sphere.lightDirection = glm::normalize(camera.pos);

    sphere.viewPos = camera.pos;
    // Find max degree for normalization
    int max_degree = 1;
    for (const auto &node : universe.graph.node_list)
    {
        max_degree = std::max(max_degree, node.degree);
    }

    for (int i = 0; i < n_nodes; i++)
    {
        Node &nd = universe.graph.node_list[i];

        // Calculate and store radius based on node degree
        float min_radius = 0.5f;
        float max_radius = 2.5f;
        nd.radius = min_radius + (max_radius - min_radius) * (float)nd.degree / max_degree;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(nd.pos.x, nd.pos.y + yloc, nd.pos.z));
        glm::vec3 scale = glm::vec3(nd.radius);
        model = glm::scale(model, scale);
        sphere.setColor(nd.color.color);
        sphere.setMVP(model, view, projection);
        sphere.draw();
    }

    // Draw walkers
    for (const Walker &walker : universe.walkers)
    {
        Node &current = universe.graph.node_list[walker.current_node];

        // Get node position as glm::vec3
        glm::vec3 node_pos = current.get_glm_pos();
        node_pos.y += yloc;

        // Calculate position on surface of node sphere
        glm::vec3 dir = glm::normalize(camera.pos - node_pos);

        // Use stored angles to position walker
        // Offset angles so walkers are distributed over the whole sphere
        float offset_angle = glm::pi<float>(); // 180 degrees, covers full sphere
        glm::vec3 random_dir = glm::normalize(
            glm::rotate(glm::mat4(1.0f), offset_angle * walker.theta, glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f), offset_angle * walker.phi, glm::vec3(1, 0, 0)) *
            glm::vec4(dir, 0.0f));

        glm::vec3 walker_pos = node_pos + random_dir * static_cast<float>(current.radius);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, walker_pos);

        // Make walker smaller than nodes
        float walker_radius = 0.2f;
        glm::vec3 scale = glm::vec3(walker_radius);
        model = glm::scale(model, scale);

        // Set walker color to yellow
        sphere.setColor(glm::vec3(1.0f, 1.0f, 0.0f));
        sphere.setMVP(model, view, projection);
        sphere.draw();
    }

    // Render sphere light source
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sphere.lightPos);
    float radius = 0.4f;
    glm::vec3 scale = glm::vec3(radius, radius, radius);
    model = glm::scale(model, scale);
    sphere.setColor(sphere.lightColor);
    sphere.setMVP(model, view, projection);
    sphere.draw();
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
        camera.update_view_mat_auto(0.1, 0);
    }
    if (key == GLFW_KEY_S)
    {
        camera.update_view_mat_auto(-0.1, 0);
    }
    if (key == GLFW_KEY_A)
    {
        camera.update_view_mat_auto(0, 0.1);
    }
    if (key == GLFW_KEY_D)
    {
        camera.update_view_mat_auto(0, -0.1);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        autoRotateX = !autoRotateX;
    }
}

struct dragging_context
{
    bool dragging = false;
    double lastMouseX, lastMouseY;
    glm::vec3 lastCameraPos, lastCameraTargetPos;
    double mouseX = 0;
    double mouseY = 0;
    float drag_sensitivity = 1;
} dragging_context;

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    dragging_context.mouseX = xpos;
    dragging_context.mouseY = ypos;

    if (dragging_context.dragging)
    {
        double dx = (xpos - dragging_context.lastMouseX) * dragging_context.drag_sensitivity;
        double dy = (ypos - dragging_context.lastMouseY) * dragging_context.drag_sensitivity;

        camera.update_view_mat_mouse(dx, dy, dragging_context.lastCameraPos, dragging_context.lastCameraTargetPos);

        dragging_context.lastMouseX = xpos;
        dragging_context.lastMouseY = ypos;
        dragging_context.lastCameraPos = camera.pos;
        dragging_context.lastCameraTargetPos = camera.target_pos;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
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
    if (button != 0)
        return;

    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        // Imgui is capturing mouse, so we shouldn't handle this input
        dragging_context.dragging = false;
        return;
    }

    if (action == 1)
    {
        dragging_context.lastMouseX = dragging_context.mouseX;
        dragging_context.lastMouseY = dragging_context.mouseY;
        dragging_context.lastCameraPos = camera.pos;
        dragging_context.lastCameraTargetPos = camera.target_pos;
        dragging_context.dragging = true;
    }
    else if (dragging_context.dragging && action == 0)
    {
        dragging_context.dragging = false;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.set_viewport(width, height);
    projection = glm::perspective(
        glm::radians<float>(60.0f), (GLfloat)width / (GLfloat)height,
        0.1f, 500.0f);
}

void imgui_update_frame()
{

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("Regenerate graph"))
                init_graph();

            if (ImGui::MenuItem("Toggle rotation"))
                autoRotateX = !autoRotateX;

            // if(ImGui::MenuItem("Keyboard settings")) {}

            imgui_context.showing_steps_window |= ImGui::MenuItem("Show steps window");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            imgui_context.showing_about |= ImGui::MenuItem("About");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (imgui_context.showing_about && !imgui_context.last_frame_showing_about)
        ImGui::OpenPopup("About");
    if (ImGui::BeginPopupModal("About", &imgui_context.showing_about, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("This application visualizes the PageRank algorithm step-by-step using OpenGL to render an interactive graph.");
        ImGui::Text("Want to contribute or explore the code?");
        ImGui::Text("Check out the project on GitHub:");
        ImGui::TextLinkOpenURL("https://github.com/el-yawd/page-rank", "https://github.com/el-yawd/page-rank");
        if (ImGui::Button("Close"))
        {
            imgui_context.showing_about = false;
        }
        ImGui::EndPopup();
    }
    imgui_context.last_frame_showing_about = imgui_context.showing_about;

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 125), ImVec2(400, 550));
    if (ImGui::Begin("Light Controller", nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
        bool changed = ImGui::SliderFloat("Light Source Height", &sphere.lightPos.y, -15, 15, "Y = %.3f");
        changed |= ImGui::SliderFloat3("Light Source Direction", &sphere.lightDirection.x, -1, 1, "%.3f");

        if (changed)
        {
            imgui_context.light_pitch = asin(sphere.lightDirection.y);
            imgui_context.light_yaw = atan2(sphere.lightDirection.x, sphere.lightDirection.z);
            imgui_context.light_at_camera = false;
        }

        changed = ImGui::SliderAngle("Light Source Pitch", &imgui_context.light_pitch, -90, 90, "%.0f degrees");
        changed |= ImGui::SliderAngle("Light Source Yaw", &imgui_context.light_yaw, -180, 180, "%.0f degrees");
        if (changed)
        {
            glm::vec3 lightDir;
            lightDir.x = cos(imgui_context.light_pitch) * sin(imgui_context.light_yaw);
            lightDir.y = sin(imgui_context.light_pitch);
            lightDir.z = cos(imgui_context.light_pitch) * cos(imgui_context.light_yaw);
            sphere.lightDirection = glm::normalize(lightDir);
            imgui_context.light_at_camera = false;
        }

        ImGui::Checkbox("Light Source At Camera", &imgui_context.light_at_camera);
        ImGui::ColorPicker3("Light Color", &sphere.lightColor.r);
    }
    ImGui::End();

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 125), ImVec2(400, 650));
    if (ImGui::Begin("General Controller", nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetWindowPos(ImVec2(500, 60), ImGuiCond_FirstUseEver);
        ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
        ImGui::SliderFloat("Repulsion", &universe.repulsion, 0, 15, "%.3f");
        ImGui::SliderFloat("Gravity", &universe.gravity, 0, 15, "%.3f");
        ImGui::SliderFloat("Spring", &universe.spring_k, 0, 15, "%.3f");
        ImGui::SliderFloat("Damping", &universe.damping, 0, 15, "%.3f");
        if (ImGui::Button("Reset Graph"))
            init_graph();
        ImGui::SameLine();
        ImGui::Checkbox("Toggle Rotation", &autoRotateX);
    }
    ImGui::End();
}

std::string get_current_executable_path()
{
#if defined(unix) || defined(__unix__) || defined(__unix)
    return std::filesystem::canonical(std::filesystem::path("/proc/self/exe")).parent_path();
#endif
#if defined(_WIN32) || defined(WIN32)
    // WARNING: Not tested on Windows
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path().string();
#endif

    return "";
}

int main()
{
    srand(time(NULL));

    cout << "### Starting Page Rank ###" << endl;

    cout << "Initializing graph..." << endl;
    init_graph();
    universe.init_walkers();

    GLFWwindow *window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return EXIT_FAILURE;

    // Set OpenGL Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    imgui_context.steps_window_start_pos = ImVec2(10, WIN_HEIGHT - 100);
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

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true); // Send GLFW context to ImGui
    ImGui_ImplOpenGL3_Init("#version 300 es");  // Set ImGui OpenGL version

    // Change ImGui default theme
    ImGui::StyleColorsDark();

    // Disable ImGui config saving
    ImGui::GetIO().IniFilename = NULL;
    ImGui::GetIO().LogFilename = NULL;

    // Import and use the font
    std::string current_path = get_current_executable_path();
    if (current_path != "")
    {
        ImFont *font = io.Fonts->AddFontFromFileTTF((current_path + "/fonts/Roboto-Regular.ttf").c_str(), 15.0f, NULL, io.Fonts->GetGlyphRangesDefault());
        if (font != nullptr)
            io.FontDefault = font;
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

            // Update walker timer
            walker_timer += timeDelta;
            if (walker_timer >= walker_update_interval)
            {
                universe.update_walkers(timeDelta);
                walker_timer = 0.0f;
            }
        }

        currentTime = glfwGetTime();

        if (autoRotateX)
        {
            camera.update_view_mat_auto(2.5, 0);
        }

        render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        imgui_update_frame();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        currentTime = glfwGetTime();
        timeAccumulator += currentTime - startTime;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}