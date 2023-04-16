cmake_minimum_required (VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

project (LearnVulkan LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)
find_program(GLSLC_PROGRAM glslc REQUIRED)

include(cmake/glm.txt)
include(cmake/imgui.txt)

add_library(SDL2 ALIAS SDL2::SDL2)

# compile all shaders in shader floder
file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/*.frag ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/*.vert)
# mkdir
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders)
# compile
foreach(SHADER ${SHADERS})
    # print
    message(STATUS "Found shaders: ${SHADER}")
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/${SHADER_NAME}.spv)
    message(STATUS "Shader output: ${SHADER_OUTPUT}")
    execute_process(COMMAND ${GLSLC_PROGRAM} ${SHADER} -o ${SHADER_OUTPUT})
endforeach()
# copy shaders
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sandbox)

# set Root directory
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# build engine
add_subdirectory(engine)

# build sandbox
add_subdirectory(sandbox)
