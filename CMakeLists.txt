# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("minimum-dominating-set" VERSION 1.0 LANGUAGES CXX)

#problem was that i already have HIGHS so support for it will dubble

# Build OR-tools dependencies.
set(BUILD_DEPS ON)

include(FetchContent)
FetchContent_Declare(
        or-tools
        GIT_REPOSITORY https://github.com/google/or-tools.git
        GIT_TAG        main
)

# After the following call, the CMake targets defined by OR-Tools
# will be defined and available to the rest of the build
FetchContent_MakeAvailable(or-tools)

#TEST
set(HIGHS_INCLUDE_DIR "/usr/local/HiGHS/src")
set(HIGHS_LIBRARY_DIR "/usr/local/HiGHS/build/lib")
set(HIGHS_CONFIG_DIR "/usr/local/HiGHS/build/")

# Include the header files
include_directories(/usr/local/htd-1.2/include)
link_directories(/usr/local/htd-1.2/lib)

# Include the directory
include_directories(${HIGHS_INCLUDE_DIR})
include_directories(${HIGHS_CONFIG_DIR})
link_directories(${HIGHS_LIBRARY_DIR})

find_package(Boost REQUIRED)

#Create a sources variable with a lik to all cpp files to compile
set(SOURCES
    src/util/logger.cpp
    src/minimum-dominating-set.cpp
    src/graph/graph_io.cpp
    src/graph/context.cpp
    src/reduce.cpp
    src/solver.cpp
    src/util/timer.cpp
        src/graph/generate_tree_decomposition.cpp
        src/graph/nice_tree_decomposition.cpp
        src/graph/tree_decomposition.cpp
        src/graph/treewidth_solver.cpp
        src/sat_solver.cpp

)

# Add source to this project's executable.
add_executable (minimum-dominating-set ${SOURCES})

target_link_libraries(minimum-dominating-set highs)
target_link_libraries(minimum-dominating-set htd)
target_link_libraries(minimum-dominating-set ortools::ortools)
