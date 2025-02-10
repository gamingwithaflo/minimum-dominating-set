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

#Create a sources variable with a lik to all cpp files to compile
set(SOURCES
    src/minimum-dominating-set.cpp
    src/graph/graph_io.cpp
    src/graph/context.cpp
    src/reduce.cpp
    src/solver.cpp
)

# Add source to this project's executable.
add_executable (minimum-dominating-set ${SOURCES})
