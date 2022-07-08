project(RemoteCompilerHost)
add_executable(RemoteCompilerHost
  "main.cpp"
  "../compressor.cpp"
  "../helper.cpp"
  #"simdjson.cpp"
)

target_link_directories(RemoteCompilerHost PUBLIC ${brotli_BINARY_DIR} ${hashmap_BINARY_DIR} ${taskflow_BINARY_DIR} ${cpr_BINARY_DIR} ${curl_BINARY_DIR} ${fmt_BINARY_DIR})
target_link_libraries(RemoteCompilerHost
    brotlienc-static
    brotlidec-static
    fmt
    cpr
    ${CMAKE_CURRENT_SOURCE_DIR}/uSockets.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/uv_a.lib
    Iphlpapi.lib
    userenv.lib
)