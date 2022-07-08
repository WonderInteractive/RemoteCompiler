project(RemoteCompiler)
cmake_minimum_required(VERSION 3.23)
include(CPM.cmake)
set(CMAKE_CXX_STANDARD 20)

CPMAddPackage(
  NAME brotli
  GITHUB_REPOSITORY google/brotli
  GIT_TAG 9801a2c5d6c67c467ffad676ac301379bb877fc3
  OPTIONS
    "BROTLI_BUNDLED_MODE OFF"
)

CPMAddPackage(
  NAME hashmap
  GITHUB_REPOSITORY greg7mdp/parallel-hashmap
  GIT_TAG 655dee936b5b65ec0468517dd6728a7ada2b2e02
)

CPMAddPackage(
  NAME taskflow
  GITHUB_REPOSITORY taskflow/taskflow
  GIT_TAG 96620cce7cf3bb67e039dad4ecd556ba61347787
  OPTIONS
    "TF_BUILD_TESTS OFF"
    "TF_BUILD_EXAMPLES OFF"
    "TF_BUILD_BENCHMARKS OFF"
   
)

CPMAddPackage(
  NAME cpr
  GITHUB_REPOSITORY libcpr/cpr
  GIT_TAG dd715f6aa9d2692d31712a9ac5662ebc943298f2
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME curl
  GITHUB_REPOSITORY curl/curl
  GIT_TAG 5121d98c22dc077d72c68e7b9436a04f55bd7249
  OPTIONS
    "CURLOPT_HTTP_CONTENT_DECODING OFF"
)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG f5cdf7cb0481afdafa7ff2f5ea640f05215d4ffb
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
)


include_directories(${brotli_SOURCE_DIR};${hashmap_SOURCE_DIR};${taskflow_SOURCE_DIR};${cpr_SOURCE_DIR}/include;${curl_SOURCE_DIR}/include;${cpr_BINARY_DIR}/cpr_generated_includes;${fmt_SOURCE_DIR}/include)
add_subdirectory(local)
add_subdirectory(remote)
add_subdirectory(GUI)