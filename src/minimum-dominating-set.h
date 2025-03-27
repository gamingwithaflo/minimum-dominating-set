// exact-dominating-set.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"
#include <boost/dynamic_bitset.hpp>

int main(int argc, char* argv[]);

void reduction_info(std::string path);

void reduction(std::string path, std::string path_td);

void output_reduced_graph(std::string path);

adjacencyListBoost create_reduced_graph(MDS_CONTEXT& mds_context, std::unordered_map<int, int>& newToOldIndex);

void component_reduction(std::string path);

std::vector<boost::dynamic_bitset<>> create_domination_vector(const adjacencyListBoost& reduced_graph);

// TODO: Reference additional headers your program requires here.
