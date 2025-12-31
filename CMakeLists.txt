cmake_minimum_required(VERSION 3.20)

# Set vcpkg toolchain BEFORE project()
if(NOT CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(BZ)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
)
FetchContent_MakeAvailable(pybind11)

FetchContent_Declare(
    threepp
    GIT_REPOSITORY https://github.com/markaren/threepp.git
    GIT_TAG master
)
set(THREEPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(THREEPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(THREEPP_WITH_AUDIO OFF CACHE BOOL "" FORCE)
set(THREEPP_USE_EXTERNAL_GLFW ON CACHE BOOL "" FORCE)
set(THREEPP_FETCH_ASSIMP OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(threepp)

FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/zpl-c/enet.git
    GIT_TAG master
)
set(ENET_TEST OFF CACHE BOOL "" FORCE)
set(ENET_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(enet)


# Find external packages
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Bullet REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(cxxopts REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)

# Gather sources
file(GLOB_RECURSE CLIENT_SOURCES "${PROJECT_SOURCE_DIR}/src/client/*.cpp")
file(GLOB_RECURSE SERVER_SOURCES "${PROJECT_SOURCE_DIR}/src/server/*.cpp")
## Separate engine sources for client and server to avoid over-linking
set(ENGINE_CLIENT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/engine/client_engine.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/mesh_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/client_network.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/render.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/input.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/gui.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/physics.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/audio.cpp
)
set(ENGINE_SERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/engine/server_engine.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/mesh_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/server_network.cpp
    ${PROJECT_SOURCE_DIR}/src/engine/components/physics.cpp
)

# Create executables
add_executable(client ${CLIENT_SOURCES} ${ENGINE_CLIENT_SOURCES})
add_executable(server ${SERVER_SOURCES} ${ENGINE_SERVER_SOURCES})

# Include directories
target_include_directories(client PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/client
    ${BULLET_INCLUDE_DIRS}
    ${enet_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/client
)
target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/server
    ${BULLET_INCLUDE_DIRS}
    ${enet_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(client PRIVATE
    glfw
    glm::glm
    ${BULLET_LIBRARIES}
    enet_static
    assimp::assimp
    OpenGL::OpenGL
    threepp::threepp
    cxxopts::cxxopts
    imgui::imgui
    spdlog::spdlog
)

target_link_libraries(server PRIVATE
    glm::glm
    ${BULLET_LIBRARIES}
    enet_static
    assimp::assimp
    pybind11::embed
    cxxopts::cxxopts
    spdlog::spdlog
)

# System libraries (platform-specific)
if (UNIX AND NOT APPLE)
    target_link_libraries(client PRIVATE pthread dl m)
    target_link_libraries(server PRIVATE pthread dl m)
elseif(APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_library(COREVIDEO CoreVideo)
    target_link_libraries(client PRIVATE ${COCOA} ${IOKIT} ${COREVIDEO})
    target_link_libraries(server PRIVATE ${COCOA})
elseif(WIN32)
    target_link_libraries(client PRIVATE ws2_32)
    target_link_libraries(server PRIVATE ws2_32)
endif()
