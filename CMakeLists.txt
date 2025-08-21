cmake_minimum_required(VERSION 3.19)
project(BZ)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS OFF)

# Gather sources
file(GLOB CLIENT_SOURCES "${PROJECT_SOURCE_DIR}/src/client/*.cpp")
file(GLOB SERVER_SOURCES "${PROJECT_SOURCE_DIR}/src/server/*.cpp")

file(GLOB IMGUI_SOURCES
    libs/include/imgui/imgui.cpp
    libs/include/imgui/imgui_draw.cpp
    libs/include/imgui/imgui_tables.cpp
    libs/include/imgui/imgui_widgets.cpp
    libs/include/imgui/backends/imgui_impl_glfw.cpp
    libs/include/imgui/backends/imgui_impl_opengl3.cpp
)

# Platform-specific library folder
if (WIN32)
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs/lib/windows)
    set(LIB_EXT .lib)
elseif(APPLE)
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs/lib/mac)
    set(LIB_EXT .a)
else()
    set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs/lib/linux)
    set(LIB_EXT .a)
endif()

# Helper macro to pick platform-specific lib files
function(prebuilt_lib out_name lib_base)
    if (WIN32)
        set(${out_name} "${LIB_DIR}/${lib_base}.lib" PARENT_SCOPE)
    else()
        set(${out_name} "${LIB_DIR}/lib${lib_base}.a" PARENT_SCOPE)
    endif()
endfunction()

# Pick library files
prebuilt_lib(ENET_LIB enet)
prebuilt_lib(THREEPP_LIB threepp)
prebuilt_lib(ASSIMP_LIB assimp)
prebuilt_lib(BULLET_DYNAMICS_LIB BulletDynamics)
prebuilt_lib(BULLET_COLLISION_LIB BulletCollision)
prebuilt_lib(BULLET_LINEAR_LIB LinearMath)

# Include directories for all libraries
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/include)

# Create executables
add_executable(client ${CLIENT_SOURCES} ${IMGUI_SOURCES})
add_executable(server ${SERVER_SOURCES})

# Include directories
target_include_directories(client PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/client
    ${INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/include/bullet
    ${CMAKE_SOURCE_DIR}/libs/include/glm
    ${CMAKE_SOURCE_DIR}/libs/include/assimp
    ${CMAKE_SOURCE_DIR}/libs/include/imgui
)
target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/server
    ${INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/include/bullet
)

# Link prebuilt compiled libraries
target_link_libraries(client PRIVATE
    ${ENET_LIB}
    ${THREEPP_LIB}
    ${ASSIMP_LIB}
    ${BULLET_DYNAMICS_LIB}
    ${BULLET_COLLISION_LIB}
    ${BULLET_LINEAR_LIB}
)

target_link_libraries(server PRIVATE
    ${ENET_LIB}
)

# System libraries for threepp/OpenGL
if (UNIX AND NOT APPLE)
    target_link_libraries(client PRIVATE GL glfw X11 pthread dl)
    target_link_libraries(client PRIVATE
        z          # zlib
        bz2        # bzip2
        lzma       # lzma/xz
        pthread    # threading
        dl         # dynamic loader
        m          # math library
    )
elseif(APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_library(COREVIDEO CoreVideo)
    target_link_libraries(client PRIVATE glfw OpenGL ${COCOA} ${IOKIT} ${COREVIDEO})
elseif(WIN32)
    target_link_libraries(client PRIVATE glfw opengl32)
endif()
