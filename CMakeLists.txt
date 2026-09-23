cmake_minimum_required(VERSION 3.20)
project(Emberstaff VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# raylib
#
# Use a copy that is already installed on the machine if CMake can find one;
# otherwise fetch the pinned release from GitHub and build it as part of this
# project. Either way the game links against the `raylib` target.
# ---------------------------------------------------------------------------
set(EMBERSTAFF_RAYLIB_TAG "6.0" CACHE STRING "raylib release tag to fetch when no installed copy is found")

find_package(raylib 6.0 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES     OFF       CACHE BOOL   "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF       CACHE BOOL   "" FORCE)
    set(PLATFORM           "Desktop" CACHE STRING "" FORCE)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        ${EMBERSTAFF_RAYLIB_TAG}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

# ---------------------------------------------------------------------------
# Game
# ---------------------------------------------------------------------------
add_executable(emberstaff src/main.cpp)
target_link_libraries(emberstaff PRIVATE raylib)

target_compile_options(emberstaff PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->"
)

if(MSVC)
    # Release builds are GUI applications (no console window); Debug keeps the
    # console so raylib's TraceLog output is visible.
    target_link_options(emberstaff PRIVATE
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS;/ENTRY:mainCRTStartup>"
    )
endif()

# ---------------------------------------------------------------------------
# Runtime content
#
# The game locates its content root at startup by probing the working
# directory, the executable's directory and its parents for an `assets/`
# folder, so a build tree inside the repository runs without any copying.
#
# `cmake --build <build> --target dist` assembles a self-contained folder
# (<build>/dist: executable + assets/ + levels/) that can be zipped and shared.
# ---------------------------------------------------------------------------
set(EMBERSTAFF_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EMBERSTAFF_DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:emberstaff>" "${EMBERSTAFF_DIST_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${EMBERSTAFF_DIST_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/levels" "${EMBERSTAFF_DIST_DIR}/levels"
    DEPENDS emberstaff
    COMMENT "Assembling a self-contained release folder in ${EMBERSTAFF_DIST_DIR}"
)
