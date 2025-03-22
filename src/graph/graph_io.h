#pragma once

#include <fstream>
#include <sstream>

#include "context.h"

namespace parse {
    adjacencyListBoost read_pace_2024(std::istream& is);

    adjacencyListBoost load_pace_2024(std::string path);

    void output_context(MDS_CONTEXT& mds_context, std::string& path);

    void output_reduced_graph_instance(adjacencyListBoost& reduced_graph, std::string& path);

    std::string getNameFile(std::string& path);

    void output_solution(std::vector<int>& solution, std::string& path);
} 
