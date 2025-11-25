cmake_minimum_required(VERSION 3.19)

project(basicAPI)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
#libcurl
set(CURL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/include")
set(CURL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/lib/libcurl.dll.a")

add_executable(main "${SRC}")

target_include_directories(main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/" ${CURL_INCLUDE_DIR})
target_link_libraries(main PRIVATE ${CURL_LIBRARY})