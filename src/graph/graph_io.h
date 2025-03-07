#pragma once

#include <fstream>
#include <sstream>

#include "context.h"

#include "tree_decomposition.h"

namespace parse {
    adjacencyListBoost read_pace_2024(std::istream& is);

    adjacencyListBoost load_pace_2024(std::string path);

    void output_context(MDS_CONTEXT& mds_context, std::string& path);

    TREE_DECOMPOSITION load_tree_decomposition(std::string path);

    TREE_DECOMPOSITION read_tree_decomposition(std::istream& is);

    std::string getNameFile(std::string& path);
} 
