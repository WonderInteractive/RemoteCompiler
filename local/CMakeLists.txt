project(RemoteCompilerClient)
add_executable(RemoteCompilerClient
  "main.cpp"
  "../compressor.cpp"
  "../helper.cpp"
  "../simplecpp.cpp"
  "../simplecpp2.cpp"
  #"simdjson.cpp"
)

include_directories(F:/llvm/include;)
target_link_directories(RemoteCompilerClient PUBLIC ${brotli_BINARY_DIR} ${hashmap_BINARY_DIR} ${taskflow_BINARY_DIR} ${cpr_BINARY_DIR} ${curl_BINARY_DIR} ${fmt_BINARY_DIR})
target_link_libraries(RemoteCompilerClient
    brotlienc-static
    brotlidec-static
    fmt
    cpr
    F:/llvm/lib/libclang.lib
)