#include <vector>
#include <string>
#include <iostream>
#include <taskflow/taskflow.hpp>
#include <cpr/cpr.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "fmt/format.h"
#include <direct.h>
#include "../compressor.h"
#include "../helper.h"
#include "efsw/efsw.h"
#include "imgui_impl_opengl3.cpp"
using namespace cpr;
tf::Executor executor;
tf::Taskflow taskflow;

//method to read a file
std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

//method to write a new file
void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::binary);
    file << content;
}

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <efsw/efsw.hpp>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
struct Module {
    std::string filename;
    std::string objname;
    float progress[8] = { 0.0000001f,0.0000001f,0.0000001f,0.0000001f,0.0000001f,0.0000001f,0.0000001f,0.0000001f };
    float time = 0.0000001f;
    int doing = 0;
    Module() {}
    Module(const std::string& _module) {
        filename = _module;
        int found = filename.find(".obj");
        objname = filename.substr(0, found + 4);
        update();
    }
    void update() {
        auto file = readFile(filename);
        doing = file.size() / 4;
        for (int i = 0; i < file.size() / 4; ++i) {
            progress[i] = ((float*)(file.data()))[i];
        }
    }
};
HashT<std::string, Module> modules;
std::string dir("C:\\test\\profile");
class UpdateListener : public efsw::FileWatchListener
{
public:
    void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override
    {
        switch (action)
        {
        case efsw::Actions::Add: {
            const auto filename2 = dir + filename;
            if (filename2.size() == 9)
                return;
            modules[filename2] = Module(filename2);
            break;
        }
        case efsw::Actions::Modified: {
            const auto filename2 = dir + filename;
            if (filename2.size() == 9)
                return;
            modules[filename2].update();
            std::cout << "Updated" << std::endl;
            break;
        }
        default:
            std::cout << "Should never happen!" << std::endl;
        }
    }
};

int main(int, char**) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }
    ImGui_ImplOpenGL3_Init(glsl_version);

    efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();

    // Create the instance of your efsw::FileWatcherListener implementation
    UpdateListener* listener = new UpdateListener();

    // Add a folder to watch, and get the efsw::WatchID
    // It will watch the /tmp folder recursively ( the third parameter indicates that is recursive )
    // Reporting the files and directories changes to the instance of the listener
    efsw::WatchID watchID = fileWatcher->addWatch("C:\\test\\profile", listener, true);
    // Start watching asynchronously the directories
    fileWatcher->watch();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        {
            static int counter = 0;
            static int active = 0;
            static int total = 0;
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
            static bool only_active = false;
            ImGui::Checkbox("Only active", &only_active);
            ImGui::Text("Active: %d", active);
            ImGui::Text("Total: %d", total);

            if (ImGui::BeginTable("table1", 2, flags))
            {
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                active = 0;
                total = 0;
                if (modules.size() > 0)
                for (auto& _module : modules)
                {
                    total++;
                    //draw rectangle
                    static u32 colors[8] = { IM_COL32(25, 225, 25, 255),IM_COL32(0, 0, 255, 255),IM_COL32(255, 255, 0, 255),IM_COL32(255, 0, 0, 255),IM_COL32(255, 0, 255, 255),IM_COL32(128, 255, 128, 255),IM_COL32(255, 128, 0, 255),IM_COL32(10, 128, 128, 255) };
                    if (_module.second.doing != 8) {
                        _module.second.time += ImGui::GetIO().DeltaTime * 1000.0f;
                        active++;
                    }
                    else if (only_active) {
                        continue;
                    }
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", _module.second.objname.data());
                    ImGui::TableSetColumnIndex(1);
                    for (int i = 0; i < 1; ++i) {
                        if (i > 0) ImGui::SameLine(0,0);
                        std::string time = "";
                        if (_module.second.doing == 8)
                            time = fmt::format("{}s", _module.second.time / 1000.0f);
                        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, colors[i]);
                        ImGui::ProgressBar(100, ImVec2(_module.second.time * 0.01f, 0.0f), time.data());
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndTable();
            }
            
            static float f = 0.0f;

            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}