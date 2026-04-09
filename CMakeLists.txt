cmake_minimum_required(VERSION 3.19)

project(scraper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(WIN32)
    message(STATUS "generating for windows")
    set(CURL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/lib/libcurl.dll.a")
    set(CURL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/include")
elseif(UNIX)
    message(STATUS "generating for Linux")
    find_package(CURL REQUIRED)
    set(CURL_LIBRARY ${CURL_LIBRARIES})
    set(CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} "${SRC}")

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/" ${CURL_INCLUDE_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CURL_LIBRARY})
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -static-libstdc++ -static-libgcc)
