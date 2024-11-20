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

### ---- HACK FOR DISABLING abort on GGML_ASSERT failures

# This is applied when cmake is run initially to create the build dir. For e.g. cmake -B build

# Define the path to your dependency
set(GGML_DIR "${CMAKE_SOURCE_DIR}/stable-diffusion.cpp/ggml")

# Function to apply the patch
function(apply_patch)
    execute_process(
        COMMAND git apply ${CMAKE_SOURCE_DIR}/patches/ggml-hack-don-t-abort-the-program-when-GGML_ASSERT-fails.patch
        WORKING_DIRECTORY ${GGML_DIR}
        RESULT_VARIABLE PATCH_RESULT
    )

    if(PATCH_RESULT EQUAL 0)
        message(STATUS "Patch applied successfully.")
    else()
        message(WARNING "Failed to apply patch. Error code: ${PATCH_RESULT}")
    endif()
endfunction()

# Apply the patch before adding the subdirectory
apply_patch()

### ---- /HACK

# Include stable-diffusion.cpp
add_subdirectory(stable-diffusion.cpp)

# Add the easy-diffusion binary
file(GLOB_RECURSE ED_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(easy-diffusion ${ED_SRC})

target_include_directories(easy-diffusion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(easy-diffusion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
target_link_libraries(easy-diffusion PRIVATE ggml)
target_link_libraries(easy-diffusion PRIVATE stable-diffusion)

# avoid compiler warnings, target Windows XP and later
add_definitions(-D_WIN32_WINNT=0x0601)

# Copy the necessary DLLs to the runtime output directory
add_custom_command(TARGET easy-diffusion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:stable-diffusion>
        $<TARGET_FILE:ggml>
        $<TARGET_FILE_DIR:easy-diffusion>
)
