// exact-dominating-set.cpp : Defines the entry point for the application.
//
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"

int main()
{
	//const char* path = "C:/Users/Flori/OneDrive/Documenten/GitHub/Exact-dominating-set/tests/complete_5_graph.gr";
	const char* path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/testing_reduction/test_reduce_neighborhood_single_vertex.gr";
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);
	adjacencyListBoost* pointerGraph = &adjLBoost;
	adjacencyListBoost& refGraph = adjLBoost;
	bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	printf("%d\n", is_planar);  // 1: planar, 0: not planar
	//get the range of vertex iterator
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);
	MDS_CONTEXT& refContext = mds_context;
	reduce::reduce_graph(refContext);
	printf("check");

	return 0;
}