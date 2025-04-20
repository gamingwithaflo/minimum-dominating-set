// exact-dominating-set.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"

int main(int argc, char* argv[]);

void reduction_info(std::string path);

void reduction(std::string path, std::string path_td);

void output_reduced_graph(std::string path);

adjacencyListBoost create_reduced_graph(MDS_CONTEXT& mds_context, std::unordered_map<int, int>& newToOldIndex);

void separate_solver(std::string path, strategy_reduction red_strategy, strategy_solver sol_strategy, strategy_reduction_scheme red_scheme_strategy);

void seperate_solver_no_components(std::string path, strategy_reduction red_strategy, strategy_solver sol_strategy);

void component_reduction(std::string path);

void create_reduced_component_subgraphs(adjacencyListBoost& reduced_graph,
                                        std::vector<std::unique_ptr<adjacencyListBoost>>& sub_sub_components,
                                        std::vector<std::unordered_map<int, int>>& sub_sub_newToOldIndex,
                                        std::unordered_map<int, int>& newToOldIndex);

void create_component_subgraphs(const std::string& path,
                                std::vector<std::unique_ptr<adjacencyListBoost>>& sub_components,
                                std::vector<std::unordered_map<int, int>>& sub_newToOldIndex);

// TODO: Reference additional headers your program requires here.
