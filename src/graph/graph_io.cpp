#pragma once
#include "graph_io.h"
#include <filesystem>
#include <boost/graph/adjacency_list.hpp>
#include <cstdlib>




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

    adjacencyListBoost load_pace_2024(std::string path) {
        std::ifstream f(path);
        if (f.fail()) {
            throw std::invalid_argument("Failed to open file");
        }
        //return read_pace_2024(f);
        return read_pace_2024(f);
    }

    void output_context(MDS_CONTEXT& mds_context, std::string& path) {
        std::string prefix = "/mnt/c/Users/Flori/OneDrive/Documenten/GitHub/minimum-dominating-set/output/reduced_"; //TODO:: TEMPORARY
        std::string name = getNameFile(path);
        std::string output_path = prefix + name;

        std::ofstream outFile(output_path);

        if (!outFile) {
            printf("error with writing to file");
            return;
        }

        //write domination
        outFile << "dominated ";
        for (int i : mds_context.dominated) {
            outFile << i << " ";
        }
        outFile << std::endl;
        //write removed
        outFile << "removed ";
        for (int i : mds_context.removed) {
            outFile << i << " ";
        }
        outFile << std::endl;
        //write ignored
        outFile << "ignored ";
        for (int i : mds_context.ignored) {
            outFile << i << " ";
        }
        outFile << std::endl;
        outFile << "excluded ";
        for (int i : mds_context.excluded) {
            outFile << i << " ";
        }
        outFile << std::endl;
        //todo: included
        outFile << "included ";
        for (int i : mds_context.included) {
            outFile << i << " ";
        }
        outFile << std::endl;
        //WRITE ADJACENCYGRAPH (Format to be determined)
        size_t num_vertices = mds_context.get_total_vertices();
        size_t num_edges = mds_context.get_total_edges();

        auto [edges_itt, edge_itt_end] = mds_context.get_edge_itt();

        outFile << "p ds " << num_vertices << " " << num_edges << std::endl;
        for (; edges_itt != edge_itt_end; ++edges_itt) {
            vertex source = mds_context.get_source_edge(*edges_itt);
            vertex target = mds_context.get_target_edge(*edges_itt);
            outFile << source << " " << target << std::endl;
        }

        outFile.close();
    }

    std::string getNameFile(std::string& path) {
        size_t lastSlash = path.find_last_of("/\\"); // Handles both '/' and '\' for cross-platform support
        return (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
    }
}