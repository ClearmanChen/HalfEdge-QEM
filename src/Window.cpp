#include "Window.h"

Window::Window(const int& glfw_width = 800, const int& glfw_height = 600)
{
    this->glfw_width = glfw_width;
    this->glfw_height = glfw_height;
    this->camera = nullptr;
    this->mesh = nullptr;
    mouse_flag = true;
    mouse_lastX = 0.f;
    mouse_lastY = 0.f;
    mouse_offsetX = 0.f;
    mouse_offsetY = 0.f;
}

Window::~Window()
{
    if (this->glfw_window != nullptr)
    {
        glfwDestroyWindow(this->glfw_window);
        glfwTerminate();
    }
    if (camera != nullptr)
    {
        delete camera;
    }
    if (mesh != nullptr)
    {
        delete mesh;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool Window::init_glfw()
{
    int glfwInitRes = glfwInit();
    if (!glfwInit())
    {
        cerr << "Unable to initialize GLFW" << endl;
        return false;
    }

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);  // Disable resizing

    // Create a GLFWwindow object that we can use for GLFW's functions
    this->glfw_window =
        glfwCreateWindow(glfw_width, glfw_height, "Surface Simplification", nullptr, nullptr);
    if (!this->glfw_window)
    {
        cerr << "Unable to create GLFW window" << endl;
        glfwTerminate();
        return false;
    }

    // Set current OpenGL context
    glfwMakeContextCurrent(this->glfw_window);
    glfwSetWindowUserPointer(this->glfw_window, this);
    glfwSetCursorPosCallback(this->glfw_window, mouse_callback);
    glfwSetScrollCallback(this->glfw_window, scroll_callback);
    glfwSetKeyCallback(this->glfw_window, key_callback);
    //glfwSetInputMode(this->glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetFramebufferSizeCallback(glfw_window, framebuffer_size_callback);

    // Load OpenGL function pointers with GLAD
    int gladInitRes = gladLoadGL();
    if (!gladInitRes) 
    {
        cerr << "Unable to initialize GLAD" << endl;
        glfwDestroyWindow(this->glfw_window);
        glfwTerminate();
        return false;
    }

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(this->glfw_window, &width, &height);
    glViewport(0, 0, width, height);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(this->glfw_window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 330 core");

    return true;
}

void Window::run()
{
    if (!init_glfw())
    {
        return;
    }

    mesh = new Mesh();
    
    mesh->load_file("resource/obj/dinosaur.2k.obj");
    //mesh->load_file("resource/obj/dinosaur.2k.obj");
    //mesh->load_file("resource/obj/sphere.obj");
    auto simp = new Simplification(mesh);
    

    Shader shader("resource/shader/simple_shader.vs", "resource/shader/simple_shader.fs");
    shader.active();
    shader.setVec3("lightPos", -10.f, 10.f, 10.f);

    camera = new Camera(0.0, 0.0, 0.9, 0.0, 1.0, 0.0, 0.0, 0.0, glfw_width, glfw_height);
    camera->setShader(shader);
    float model_radius = 40;
    camera->setPose(-1.30951*model_radius, 1.83029*model_radius, 1.05599*model_radius, -37.5001, -49.8);
    camera->setFix(false);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    float lastFrame = 0.0f;
    double start_time = 0.0f;
    double run_time = 0.0f;
    bool is_run = false;
    bool dynamic_dis = false;

    while (!glfwWindowShouldClose(this->glfw_window)) 
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Setting"); 
            ImGui::Text("Mesh simplication");
            ImGui::SliderFloat("Reduction Ratio", &f, 0.0f, 1.0f);
            ImGui::Checkbox("Dynamic", &dynamic_dis); ImGui::SameLine();
            if (ImGui::Button("Run"))
            {
                simp->init(f);
                is_run = true;
                start_time = glfwGetTime();
            }
            if (is_run)
            {
                if (!simp->is_finished())
                {
                    simp->step();
                    run_time = glfwGetTime() - start_time;
                }
                else
                    is_run = false;
                if(dynamic_dis)mesh->updateGL();
            }
            ImGui::ProgressBar(simp->rate_of_process());
            ImGui::Text("Mesh num = %d, Total Cost = %.4f", mesh->m_faces.size(), simp->total_cost);
            ImGui::Text("Time = %.3lfs", run_time);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera->setShader(shader);
        mesh->draw(shader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(this->glfw_window);
    }

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Window::processInput()
{
    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfw_window, true);
    if (glfwGetKey(glfw_window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(glfw_window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(glfw_window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(glfw_window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
}

//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    // make sure the viewport matches the new window dimensions; note that width and 
//    // height will be significantly larger than specified on retina displays.
//    glViewport(0, 0, width, height);
//}
//
// 
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* win_obj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (win_obj->mouse_flag)
        {
            win_obj->mouse_lastX = xpos;
            win_obj->mouse_lastY = ypos;
            win_obj->mouse_flag = false;
        }

        win_obj->mouse_offsetX = xpos - win_obj->mouse_lastX;
        win_obj->mouse_offsetY = win_obj->mouse_lastY - ypos; // reversed since y-coordinates go from bottom to top

        win_obj->mouse_lastX = xpos;
        win_obj->mouse_lastY = ypos;

        win_obj->camera->ProcessMouseMovement(win_obj->mouse_offsetX, win_obj->mouse_offsetY);
    }
    else
    {
        win_obj->mouse_flag = true;
    }
}
//
//// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* win_obj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    win_obj->camera->ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        Window* win_obj = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win_obj->camera->printState();
    }
}