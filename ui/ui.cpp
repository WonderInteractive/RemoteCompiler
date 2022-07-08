#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengles2.h>
#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui_impl_sdl.cpp"
//#include "imgui_impl_opengl3.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

SDL_Window*     g_Window = NULL;
SDL_GLContext   g_GLContext = NULL;
uint64_t tick = 0;
void ui_loop(void* arg) {
	ImGuiIO& io = ImGui::GetIO();
    IM_UNUSED(arg); // We can pass this argument as the second parameter of emscripten_set_main_loop_arg(), but we don't use that.

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool show_demo_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        // Capture events here, based on io.WantCaptureMouse and io.WantCaptureKeyboard
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
	ImGui::SetWindowFontScale(0.25);
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGui::SetNextWindowSize({ io.DisplaySize.x, io.DisplaySize.y });
        ImGui::SetNextWindowPos({ 0, 0 });

        ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

        ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state

        ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

        if (ImGui::Button("Load GameInfo")) {
			_fs.FetchGameInfo();
		}
		if (ImGui::Button("Load GameDebug")) {
			_fs.FetchGameDebug();
		}
		if (ImGui::Button("FetchAll - Cloudflare")) {
			downloaded[0].clear();
			fetchAll();
		}
        ImGui::SameLine();
        ImGui::Text("downloaded = %d", downloaded[0].size());
        ImGui::SameLine();
        ImGui::Text("Time Taken = %f", total_time[0]);

		if (ImGui::Button("FetchAll - AWS")) {
			downloaded[1].clear();
			fetchAllAWS();
		}
        ImGui::SameLine();
        ImGui::Text("downloaded = %d", downloaded[1].size());
        ImGui::SameLine();
        ImGui::Text("Time Taken = %f", total_time[1]);

		if (ImGui::Button("GetAll - Cloudflare")) {
			downloaded[0].clear();
			getAll();
		}
        ImGui::SameLine();
        ImGui::Text("downloaded = %d", downloaded[0].size());
        ImGui::SameLine();
        ImGui::Text("Time Taken = %f", total_time[0]);

		if (ImGui::Button("GetAllAsyncify - Cloudflare")) {
			downloaded[1].clear();
			getAllAsyncify();
		}
        ImGui::SameLine();
        ImGui::Text("downloaded = %d", downloaded[1].size());
        ImGui::SameLine();
        ImGui::Text("Time Taken = %f", total_time[1]);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//static std::vector<char> selected(_fs.gameInfo.fnhash_to_datahash.size());
		static bool selected[1000] = {};
		//auto colums = io.DisplaySize.x / 100; //when doing hashes
		auto colums = 64; //when doing progress
		if (_fs.gameInfo.fnhash_to_datahash.size() > 0) {
			if (ImGui::BeginTable("split1", colums, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
			{
				int i = 0;
				for (auto v : _fs.gameInfo.fnhash_to_datahash)
				{
					auto _fnhash = v.first;
					auto _datahash = v.second;
					//auto _fn = _fs.gameInfo.fnhash_to_fn[_fnhash];
					ImGui::TableNextColumn();
					//ImGui::Selectable(std::to_string(_datahash).data(), downloaded[1].contains(_datahash));
					//ImGuiCol_Border
					ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255,255,0,255));
					ImGui::Selectable(" ", downloaded[0].contains(_datahash));
					ImGui::PopStyleColor();
				}
				ImGui::EndTable();
			}
			//ImGui::SameLine();
			if (ImGui::BeginTable("split1", colums, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
			{
				int i = 0;
				for (auto v : _fs.gameInfo.fnhash_to_datahash)
				{
					auto _fnhash = v.first;
					auto _datahash = v.second;
					//auto _fn = _fs.gameInfo.fnhash_to_fn[_fnhash];
					ImGui::TableNextColumn();
					ImGui::Selectable(" ", downloaded[1].contains(_datahash));
				}
				ImGui::EndTable();
			}
		}
		ImGui::Spacing();
		if (_fs.gameInfo.fnhash_to_fn.size() > 0) {
			if (ImGui::BeginTable("split2", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
				for (auto v : _fs.gameInfo.fnhash_to_fn) {
					auto hash = std::to_string(v.first);
					auto label = v.second;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Selectable(label.data(), false, ImGuiSelectableFlags_SpanAllColumns);
					ImGui::TableNextColumn();
					ImGui::Text(hash.data());
					ImGui::TableNextColumn();
					ImGui::Text("123456");
				}
				ImGui::EndTable();
			}
		}
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(g_Window, g_GLContext);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_Window);
}
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
int startUI() {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // For the browser using Emscripten, we are going to use WebGL1 with GL ES2. See the Makefile. for requirement details.
    // It is very likely the generated file won't work in many browsers. Firefox is the only sure bet, but I have successfully
    // run this code on Chrome for Android for example.
    const char* glsl_version = "#version 100";
    //const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_Window = SDL_CreateWindow("Dear ImGui Emscripten example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    g_GLContext = SDL_GL_CreateContext(g_Window);
    if (!g_GLContext)
    {
        fprintf(stderr, "Failed to initialize WebGL context!\n");
        return 1;
    }
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    emscripten_set_main_loop_arg(ui_loop, NULL, 0, true);
}