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

const std::string LOREM_IPSUM = "Lorem ipsum dolor sit amet consectetur adipiscing elit. Blandit quis suspendisse aliquet nisi sodales consequat magna. Sem placerat in id cursus mi pretium tellus. Finibus facilisis dapibus etiam interdum tortor ligula congue. Sed diam urna tempor pulvinar vivamus fringilla lacus. Porta elementum a enim euismod quam justo lectus. Nisl malesuada lacinia integer nunc posuere ut hendrerit. Imperdiet mollis nullam volutpat porttitor ullamcorper rutrum gravida. Ad litora torquent per conubia nostra inceptos himenaeos. Ornare sagittis vehicula praesent dui felis venenatis ultrices. Dis parturient montes nascetur ridiculus mus donec rhoncus. Potenti ultricies habitant morbi senectus netus suscipit auctor. Maximus eget fermentum odio phasellus non purus est. Platea dictumst lorem ipsum dolor sit amet consectetur. Dictum risus blandit quis suspendisse aliquet nisi sodales. Vitae pellentesque sem placerat in id cursus mi. Luctus nibh finibus facilisis dapibus etiam interdum tortor. Eu aenean sed diam urna tempor pulvinar vivamus. Tincidunt nam porta elementum a enim euismod quam. Iaculis massa nisl malesuada lacinia integer nunc posuere. Velit aliquam imperdiet mollis nullam volutpat porttitor ullamcorper. Taciti sociosqu ad litora torquent per conubia nostra. Primis vulputate ornare sagittis vehicula praesent dui felis. Et magnis dis parturient montes nascetur ridiculus mus. Accumsan maecenas potenti ultricies habitant morbi senectus netus. Mattis scelerisque maximus eget fermentum odio phasellus non. Hac habitasse platea dictumst lorem ipsum dolor sit. Vestibulum fusce dictum risus blandit quis suspendisse aliquet. Ex sapien vitae pellentesque sem placerat in id. Neque at luctus nibh finibus facilisis dapibus etiam. Tempus leo eu aenean sed diam urna tempor. Viverra ac tincidunt nam porta elementum a enim. Bibendum egestas iaculis massa nisl malesuada lacinia integer. Arcu dignissim velit aliquam imperdiet mollis nullam volutpat. Class aptent taciti sociosqu ad litora torquent per. Turpis fames primis vulputate ornare sagittis vehicula praesent. Natoque penatibus et magnis dis parturient montes nascetur. Feugiat tristique accumsan maecenas potenti ultricies habitant morbi. Nulla molestie mattis scelerisque maximus eget fermentum odio. Cubilia curae hac habitasse platea dictumst lorem ipsum. Mauris pharetra vestibulum fusce dictum risus blandit quis. Quisque faucibus ex sapien vitae pellentesque sem placerat. Ante condimentum neque at luctus nibh finibus facilisis. Duis convallis tempus leo eu aenean sed diam. Sollicitudin erat viverra ac tincidunt nam porta elementum. Nec metus bibendum egestas iaculis massa nisl malesuada. Commodo augue arcu dignissim velit aliquam imperdiet mollis. Semper vel class aptent taciti sociosqu ad litora. Cras eleifend turpis fames primis vulputate ornare sagittis. Orci varius natoque penatibus et magnis dis parturient. Proin libero feugiat tristique accumsan maecenas potenti ultricies. Eros lobortis nulla molestie mattis scelerisque maximus eget. Curabitur facilisi cubilia curae hac habitasse platea dictumst. Efficitur laoreet mauris pharetra vestibulum fusce dictum risus. Adipiscing elit quisque faucibus ex sapien vitae pellentesque. Consequat magna ante condimentum neque at luctus nibh. Pretium tellus duis convallis tempus leo eu aenean. Ligula congue sollicitudin erat viverra ac tincidunt nam. Fringilla lacus nec metus bibendum egestas iaculis massa. Justo lectus commodo augue arcu dignissim velit aliquam. Ut hendrerit semper vel class aptent taciti sociosqu. Rutrum gravida cras eleifend turpis fames primis vulputate. Inceptos himenaeos orci varius natoque penatibus et magnis. Venenatis ultrices proin libero feugiat tristique accumsan maecenas. Donec rhoncus eros lobortis nulla molestie mattis scelerisque. Suscipit auctor curabitur facilisi cubilia curae hac habitasse. Purus est efficitur laoreet mauris pharetra vestibulum fusce. Amet consectetur adipiscing elit quisque faucibus ex sapien. Nisi sodales consequat magna ante condimentum neque at. Cursus mi pretium tellus duis convallis tempus leo. Interdum tortor ligula congue sollicitudin erat viverra ac. Pulvinar vivamus fringilla lacus nec metus bibendum egestas. Euismod quam justo lectus commodo augue arcu dignissim. Nunc posuere ut hendrerit semper vel class aptent. Porttitor ullamcorper rutrum gravida cras eleifend turpis fames. Conubia nostra inceptos himenaeos orci varius natoque penatibus. Dui felis venenatis ultrices proin libero feugiat tristique. Ridiculus mus donec rhoncus eros lobortis nulla molestie. Senectus netus suscipit auctor curabitur facilisi cubilia curae. Phasellus non purus est efficitur laoreet mauris pharetra. Dolor sit amet consectetur adipiscing elit quisque faucibus. Suspendisse aliquet nisi sodales consequat magna ante condimentum. In id cursus mi pretium tellus duis convallis. Dapibus etiam interdum tortor ligula congue sollicitudin erat. Urna tempor pulvinar vivamus fringilla lacus nec metus. Aenim euismod quam justo lectus commodo augue. Lacinia integer nunc posuere ut hendrerit semper vel. Nullam volutpat porttitor ullamcorper rutrum gravida cras eleifend. Torquent per conubia nostra inceptos himenaeos orci varius. Vehicula praesent dui felis venenatis ultrices proin libero. Montes nascetur ridiculus mus donec rhoncus eros lobortis. Habitant morbi senectus netus suscipit auctor curabitur facilisi. Fermentum odio phasellus non purus est efficitur laoreet. Lorem ipsum dolor sit amet consectetur adipiscing elit. Blandit quis suspendisse aliquet nisi sodales consequat magna. Sem placerat in id cursus mi pretium tellus. Finibus facilisis dapibus etiam interdum tortor ligula congue. Sed diam urna tempor pulvinar vivamus fringilla lacus. Porta elementum a enim euismod quam justo lectus. Nisl malesuada lacinia integer nunc posuere ut hendrerit.";
const int LAST_STEP = 100; // only for testing, probably this won't be known at compile-time
struct pagerank_step_context
{
    int current_step = 0;
    int total_steps = LAST_STEP;

    // could be a function that lazy-loads the description instead
    std::vector<std::string> step_description;
} step;

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
        std::function<void(int)> dfs = [&](int node) {
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

    for (int i = 0; i < n_nodes; i++)
    {
        Node nd = universe.graph.node_list[i];

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(nd.pos.x, nd.pos.y + yloc, nd.pos.z));
        float radius = 1.0f;

        if (show_degree)
        {
            radius = min(nd.degree / 2.5, 1.5);
        }

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

struct imgui_context
{
    bool showing_about = false;
    bool last_frame_showing_about = false;
    ImVec2 steps_window_start_pos;
    bool showing_steps_window = true;
} imgui_context;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.set_viewport(width, height);
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

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 125), ImVec2(300, 250));
    if (imgui_context.showing_steps_window && ImGui::Begin("PageRank Step Descriptor", &imgui_context.showing_steps_window, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::SetWindowPos(imgui_context.steps_window_start_pos, ImGuiCond_FirstUseEver);
        ImGui::Text(std::format("Step: {}/{}", step.current_step, step.total_steps).c_str());
        ImGui::TextWrapped(step.step_description[step.current_step].c_str());
        ImGui::Dummy(ImVec2(0, 10));

        if (ImGui::Button("Prev") && step.current_step > 0)
            step.current_step--;
        ImGui::SameLine();
        if (ImGui::Button("Next") && step.current_step < step.total_steps)
            step.current_step++;

        ImGui::End();
    }
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
    // Only for testing, we'll have to do it another way!
    step.step_description.resize(step.total_steps);
    for (int i = 0; i < step.total_steps; i++)
        step.step_description[i] = LOREM_IPSUM.substr(rand() % (LOREM_IPSUM.size() - 60), 60);

    cout << "### Starting Page Rank ###" << endl;

    cout << "Initializing graph..." << endl;
    init_graph();

    GLFWwindow *window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return EXIT_FAILURE;

    // Set OpenGL Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
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
    (void)io;
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
        }

        currentTime = glfwGetTime();

        if (autoRotateX)
        {
            camera.update_view_mat(2.5, 0);
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