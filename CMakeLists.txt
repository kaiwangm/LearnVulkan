cmake_minimum_required (VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project (VulkanTutorial LANGUAGES CXX)

include(cmake/FindVulkan.cmake)
include(cmake/FindSDL2.cmake)
include(cmake/CopyFiles.cmake)


aux_source_directory(engine Engine)

add_library(engine STATIC ${Engine})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_compile_features(engine PUBLIC cxx_std_17)

add_subdirectory(sandbox)