cmake_minimum_required(VERSION 3.19)

project(WfMarketScraper)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(WIN32)
    message(STATUS "compiling for windows")
    set(CURL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/lib/libcurl.dll.a")
    set(CURL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendors/curl-8.16.0_2-win64-mingw/include")
elseif(UNIX)
    message(STATUS "compiling for windows")
    find_package(CURL REQUIRED)
    set(CURL_LIBRARY ${CURL_LIBRARIES})
    set(CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS})
endif()
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
#libcurl

add_executable(main "${SRC}")

target_include_directories(main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/" ${CURL_INCLUDE_DIR})
target_link_libraries(main PRIVATE ${CURL_LIBRARY})
