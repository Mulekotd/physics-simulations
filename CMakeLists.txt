cmake_minimum_required(VERSION 3.14)

project(physics-simulations LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_policy(SET CMP0072 NEW)

include(FetchContent)

# --------------------------------------------------------------------------- #
# 1. Source files
# --------------------------------------------------------------------------- #

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

list(APPEND SRC ${CMAKE_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${SRC}
    include/engine/Ray.hpp
    src/engine/TextureManager.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# --------------------------------------------------------------------------- #
# 2. GLM
# --------------------------------------------------------------------------- #

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# --------------------------------------------------------------------------- #
# 3. OpenGL
# --------------------------------------------------------------------------- #

find_package(OpenGL REQUIRED)

target_compile_definitions(${PROJECT_NAME} PRIVATE GL_GLEXT_PROTOTYPES)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# --------------------------------------------------------------------------- #
# 4. GLFW
# --------------------------------------------------------------------------- #

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW3 REQUIRED glfw3)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GLFW3_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${GLFW3_LIBRARIES}
    )
else()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )

    FetchContent_MakeAvailable(glfw)

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()

# --------------------------------------------------------------------------- #
# 5. Dear ImGui
# --------------------------------------------------------------------------- #

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

if(GLFW3_FOUND)
    target_link_libraries(imgui PUBLIC
        ${GLFW3_LIBRARIES}
        OpenGL::GL
    )
else()
    target_link_libraries(imgui PUBLIC
        glfw
        OpenGL::GL
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --------------------------------------------------------------------------- #
# 6. Linux extra libs
# --------------------------------------------------------------------------- #

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dl
        m
    )
endif()

# --------------------------------------------------------------------------- #
# 7. macOS frameworks
# --------------------------------------------------------------------------- #

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()
