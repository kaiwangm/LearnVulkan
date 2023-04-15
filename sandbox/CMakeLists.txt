add_executable(sandbox)
target_sources(sandbox PRIVATE ./sandbox.cpp)
target_link_libraries(sandbox PUBLIC engine SDL2 imgui)
target_include_directories(sandbox PUBLIC ${ROOT_DIR})