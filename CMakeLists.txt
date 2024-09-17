cmake_minimum_required (VERSION 3.8)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

project("BPM")

########## SOURCE FILES ##########
#src
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.c")
#imgui#
file(GLOB IMGUI_SOURCES external/imgui/*.cpp)
list(APPEND IMGUI_SOURCES "external/imgui/backends/imgui_impl_glfw.cpp")
list(APPEND IMGUI_SOURCES "external/imgui/backends/imgui_impl_opengl3.cpp")
list(APPEND IMGUI_SOURCES "external/imgui/misc/cpp/imgui_stdlib.cpp")

########## EXECUTABLE ##########
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${IMGUI_SOURCES} "${PROJECT_SOURCE_DIR}/external/glad.c" "${PROJECT_SOURCE_DIR}/external/stb_image.cpp")

# Configure a header file to pass the assets directory to the source code
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
configure_file("${PROJECT_SOURCE_DIR}/config/PathConfig.h.in" "${CMAKE_BINARY_DIR}/config/PathConfig.h")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/config")
########## LINK LIBRARIES ##########
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/libs CACHE PATH "Missing description." FORCE)
# GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

########## INCLUDE  ##########
target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PUBLIC "external")
#imgui#
target_include_directories(${PROJECT_NAME} PUBLIC "external/imgui")
target_include_directories(${PROJECT_NAME} PUBLIC "external/imgui/backends")
target_include_directories(${PROJECT_NAME} PUBLIC "external/imgui/misc/cpp")


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
endif()
