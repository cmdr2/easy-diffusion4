cmake_minimum_required(VERSION 3.16)
project(easy-diffusion)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define output directories for all build artifacts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set flags for shared library builds
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build ggml as a shared library")
set(SD_BUILD_SHARED_LIBS ON CACHE BOOL "Build stable-diffusion as a shared library")
set(SD_BUILD_SHARED_GGML_LIB ON CACHE BOOL "Build ggml as a separate shared library")

# Include stable-diffusion.cpp
add_subdirectory(stable-diffusion.cpp)

# Add the easy-diffusion binary
file(GLOB_RECURSE ED_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(easy-diffusion ${ED_SRC})

target_include_directories(easy-diffusion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(easy-diffusion PRIVATE ggml)
target_link_libraries(easy-diffusion PRIVATE stable-diffusion)

# Copy the necessary DLLs to the runtime output directory
add_custom_command(TARGET easy-diffusion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:stable-diffusion>
        $<TARGET_FILE:ggml>
        $<TARGET_FILE_DIR:easy-diffusion>
)
