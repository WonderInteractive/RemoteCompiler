project(RemoteCompilerClientGUI)
add_executable(RemoteCompilerClientGUI
  "main.cpp"
  "../compressor.cpp"
  "../helper.cpp"
  #"../ui/imgui_impl_opengl3.cpp"
  "../ui/imgui_impl_glfw.cpp"
  "../ui/imgui_widgets.cpp"
  "../ui/imgui_tables.cpp"
  "../ui/imgui.cpp"
  "../ui/imgui_draw.cpp"
  "../ui/imgui_demo.cpp"
  "../src/gl.c"
)

CPMAddPackage(
  NAME efsw
  GITHUB_REPOSITORY SpartanJ/efsw
  GIT_TAG b60a4e6eff88999e122a50b33dbdf61a97bc5969
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "BUILD_TEST_APP OFF"
    "VERBOSE OFF"
    "NO_ATOMICS ON"
    "EFSW_INSTALL OFF"
)

CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG d3ede7b6847b66cf30b067214b2b4b126d4c729b
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
)



include_directories(${esfw_SOURCE_DIR};${glfw_SOURCE_DIR};${CMAKE_SOURCE_DIR}/ui;${CMAKE_SOURCE_DIR}/include;)

target_link_directories(RemoteCompilerClientGUI PUBLIC ${brotli_BINARY_DIR} ${hashmap_BINARY_DIR} ${taskflow_BINARY_DIR} ${cpr_BINARY_DIR} ${curl_BINARY_DIR} ${fmt_BINARY_DIR} ${efsw_BINARY_DIR} ${glfw_BINARY_DIR})
target_link_libraries(RemoteCompilerClientGUI
    brotlienc-static
    brotlidec-static
    fmt
    cpr
    efsw
    glfw
    opengl32
)