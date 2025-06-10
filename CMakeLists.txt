# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.28.3)

project ("pace2025" VERSION 1.0 LANGUAGES CXX)

# Ensure we're setting optimization for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -flto")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#problem was that i already have HIGHS so support for it will dubble

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/external/htd/include)

# Link directories (discouraged in modern CMake, but still possible)
#link_directories(${CMAKE_SOURCE_DIR}/external/htd/lib)

set(CMAKE_BUILD_RPATH "") # Initialize as empty list

# Add the path for htd library
list(APPEND CMAKE_BUILD_RPATH "$ORIGIN/external/htd/lib")

# Add the path for ortools library
# Assuming ortools places its libraries in 'lib' directly under its prefix or build dir.
# If ortools was built by you into CMAKE_BINARY_DIR, you might use:
list(APPEND CMAKE_BUILD_RPATH "$ORIGIN/external/or-tools/lib")

message(STATUS "DEBUG: CMAKE_BUILD_RPATH is set to: ${CMAKE_BUILD_RPATH}") # <-- Add this line

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../external/htd/lib;\#ORIGIN/../external/ortools/lib/")

# Ensure RPATH is not stripped during installation
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # Add paths from linked libraries to RPATH
set(CMAKE_SKIP_INSTALL_RPATH FALSE) # Don't skip setting RPATH

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/or-tools/")

find_package(Boost REQUIRED)

find_package(ortools REQUIRED)

# --- Find and import the HTD library to create a proper target ---
# This step is critical for embedding RPATH.
# Use 'NAMES' to search for both common linker names and the specific versioned name.
find_library(HTD_LIB
    NAMES htd htd.so.0 # Search for libhtd.so (from 'htd') and libhtd.so.0 directly
    PATHS ${CMAKE_SOURCE_DIR}/external/htd/lib/ # Only search in your specified path
    NO_DEFAULT_PATH # Do not search default system locations
)

# Crucial check: If HTD_LIB is NOT found, stop the build.
if (NOT HTD_LIB)
    message(FATAL_ERROR "libhtd.so or libhtd.so.0 not found in ${CMAKE_SOURCE_DIR}/external/htd/lib. Please ensure the file exists and has correct permissions.")
endif()

add_library(htd_imported_lib SHARED IMPORTED)
set_target_properties(htd_imported_lib PROPERTIES IMPORTED_LOCATION "${HTD_LIB}")

#Create a sources variable with a lik to all cpp files to compile
set(SOURCES
    src/util/logger.cpp
    src/minimum-dominating-set.cpp
    src/graph/graph_io.cpp
    src/graph/context.cpp
    src/reduce.cpp
    src/util/timer.cpp
        src/graph/generate_tree_decomposition.cpp
        src/graph/nice_tree_decomposition.cpp
        src/graph/tree_decomposition.cpp
        src/graph/treewidth_solver.cpp
        src/sat_solver.cpp

)

# Add source to this project's executable.
add_executable (pace2025 ${SOURCES})

target_link_libraries(pace2025 PRIVATE htd_imported_lib ortools::ortools)

