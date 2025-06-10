## INSTALL RTS-PACE2025
This file will walk you through installing the necessary dependenies and tools for building and running the RTS minimum dominating set solver.


### Prerequisites
- Linux distribution (tested under Ubuntu 24.04)
- CMake >= 3.28.3
- C++ compiler supporting c++17.

### Dependencies
- OR-tools (https://developers.google.com/optimization/install/cpp/binary_linux)
OR-Tools is an open source software suite for optimization, tuned for tackling the world's toughest problems in vehicle routing, flows, integer and linear programming, and constraint programming.

After modeling your problem in the programming language of your choice, you can use any of a half dozen solvers to solve it: commercial solvers such as Gurobi or CPLEX, or open-source solvers such as SCIP, GLPK, or Google's GLOP and award-winning CP-SAT.

- htd (https://github.com/mabseher/htd)
a small but efficient c++ libary for computing tree decompositions.

- Boost (https://www.boost.org/) 

If OR-tools is already installed in the standard system paths on your machine, you can remove the ./external/OR-tools directory, which includes prebuilt versions of OR-Tools compiled for Ubuntu 24.04.

IMPORTANT: no prebuilt version of Boost is provided.


### Build the executable.

create a folder named build at the root.
```
mkdir build
cd build
```

call cmake in the build build folder
```
cmake ..
```

build the executable
```
make
```

### Run the executable
./pace2025 < exact_018.gr



