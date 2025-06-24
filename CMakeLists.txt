cmake_minimum_required(VERSION 3.10)
project(BZ)

set(CMAKE_CXX_STANDARD 17)
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


# Add dependencies
add_subdirectory(libs/raylib)
add_subdirectory(libs/enet)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cxxopts)

# Create client executable
add_executable(client ${CLIENT_SOURCES})

# Create server executable
add_executable(server ${SERVER_SOURCES})


# Include dirs for both
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/src/client)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/src/server)

# Link libraries for both
target_link_libraries(client PRIVATE raylib enet::enet_static spdlog::spdlog cxxopts::cxxopts)
target_link_libraries(server PRIVATE raylib enet::enet_static spdlog::spdlog cxxopts::cxxopts)