cmake_minimum_required (VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project (VulkanTutorial LANGUAGES CXX)

include(cmake/FindVulkan.cmake)
include(cmake/FindSDL2.cmake)
include(cmake/CopyFiles.cmake)

find_program(GLSLC_PROGRAM glslc REQUIRED)
# compile all shaders in shader floder
file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_LIST_DIR}/shaders/*.frag ${CMAKE_CURRENT_LIST_DIR}/shaders/*.vert)
# mkdir
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
# compile
foreach(SHADER ${SHADERS})
    # print
    message(STATUS "Found shaders: ${SHADER}")
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv)
    message(STATUS "Shader output: ${SHADER_OUTPUT}")
    execute_process(COMMAND ${GLSLC_PROGRAM} ${SHADER} -o ${SHADER_OUTPUT})
endforeach()

aux_source_directory(engine Engine)

add_library(engine STATIC ${Engine})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_compile_features(engine PUBLIC cxx_std_17)

add_subdirectory(sandbox)
# copy shaders
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sandbox)
