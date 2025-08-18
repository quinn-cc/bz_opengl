cmake_minimum_required(VERSION 3.19)
project(BZ)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

set(BUILD_SHARED_LIBS OFF)

# Gather sources for client
file(GLOB CLIENT_SOURCES
    "${PROJECT_SOURCE_DIR}/src/client/*.cpp"
)

# Gather sources for server
file(GLOB SERVER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/server/*.cpp"
)

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add dependencies
add_subdirectory(libs/enet)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cxxopts)
add_subdirectory(libs/glm)
add_subdirectory(libs/threepp)
add_subdirectory(libs/assimp)
add_subdirectory(libs/bullet)

# Create client executable
add_executable(client ${CLIENT_SOURCES})

# Create server executable
add_executable(server ${SERVER_SOURCES})

target_compile_features(client PRIVATE cxx_std_20)
target_compile_features(server PRIVATE cxx_std_20)

target_include_directories(client PRIVATE
    libs/bullet/src
)

# Include dirs for both
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/src/client)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/src/server)

# Link libraries for both
target_link_libraries(client PRIVATE enet::enet_static spdlog::spdlog cxxopts::cxxopts glm::glm threepp assimp BulletDynamics BulletCollision LinearMath)
target_link_libraries(server PRIVATE enet::enet_static spdlog::spdlog cxxopts::cxxopts glm::glm)