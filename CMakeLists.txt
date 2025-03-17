# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("minimum-dominating-set")

set(HIGHS_INCLUDE_DIR "/usr/local/HiGHS/src")
set(HIGHS_LIBRARY_DIR "/usr/local/HiGHS/build/lib")
set(HIGHS_CONFIG_DIR "/usr/local/HiGHS/build/")


# Include the directory
include_directories(${HIGHS_INCLUDE_DIR})
include_directories(${HIGHS_CONFIG_DIR})
link_directories(${HIGHS_LIBRARY_DIR})

find_package(Boost REQUIRED)

include_directories("/mnt/c/HiGHS/src")

#Create a sources variable with a lik to all cpp files to compile
set(SOURCES
    src/util/logger.cpp
    src/minimum-dominating-set.cpp
    src/graph/graph_io.cpp
    src/graph/context.cpp
    src/graph/tree_decomposition.cpp
    src/reduce.cpp
    src/solver.cpp
    src/util/timer.cpp

)

# Add source to this project's executable.
add_executable (minimum-dominating-set ${SOURCES})

target_link_libraries(minimum-dominating-set highs)
