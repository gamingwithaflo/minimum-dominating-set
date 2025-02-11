// exact-dominating-set.cpp : Defines the entry point for the application.
//
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"
#include "solver.h"

int main()
{
	//std::string path = "C:/Users/Flori/OneDrive/Documenten/GitHub/Exact-dominating-set/tests/complete_5_graph.gr";
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/testing_reduction/test_reduce_neighborhood_pair_vertex3.gr";
	std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/T1Pace/dat_200_500_1.gr";
	std::string name = parse::getNameFile(path);
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);
	adjacencyListBoost* pointerGraph = &adjLBoost;
	adjacencyListBoost& refGraph = adjLBoost;
	bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	printf("%d\n", is_planar);  // 1: planar, 0: not planar
	//get the range of vertex iterator
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);
	MDS_CONTEXT& refContext = mds_context;
	operations_research::solve_dominating_set(mds_context);
	reduce::reduce_graph(refContext);
	operations_research::solve_dominating_set(refContext);
	mds_context.update_vertices();
	int total_vertices = mds_context.get_total_vertices();
	return 0;
}