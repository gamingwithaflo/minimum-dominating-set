#pragma once
#include "graph_io.h"


namespace parse {
    adjacencyListBoost construct_AdjacencyList_Boost(int n, std::vector<std::pair<int, int>> const& edges) {
        adjacencyListBoost g(n);
        for (auto& p : edges) {
            boost::add_edge(p.first, p.second, g);
        }
        return g;
    }
    
    adjacencyListBoost read_pace_2024(std::istream& is) {
        std::vector<std::pair<int, int>> edges;

        int i = 0;
        int n; // 
        long long m;
        int u, v;
        std::string s;
        
        //Assumption : no spaces before the first letter.
        for (std::string line; std::getline(is, line);) {
            if (line.empty()) continue;

            if (line[i] == 'c') continue;  // ignore comments

            auto ss = std::stringstream(line);
            if (line[i] == 'p') {
                ss >> s >> s >> n >> m;
            } else {
                ss >> u >> v;
                edges.push_back({ u - 1, v - 1 });
            }
        }
        return construct_AdjacencyList_Boost(n, edges);
    }

    adjacencyListBoost load_pace_2024(char const* path) {
        std::ifstream f(path);
        if (f.fail()) {
            throw std::invalid_argument("Failed to open file");
        }
        //return read_pace_2024(f);
        return read_pace_2024(f);
    }

    void output_pace_2024(MDS_CONTEXT& mds_context) {

    }

    const char* getNameFile(const char* path) {
        char* name;
        const char* lastSlash = strrchr(path, '/'); // Find last occurrence of '/'
        const char* name_with_extention = (lastSlash) ? lastSlash + 1 : path; // Return substring after last '/'
        return name_with_extention;        
    }
}