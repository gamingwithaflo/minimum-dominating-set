// exact-dominating-set.cpp : Defines the entry point for the application.
//
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"
#include "solver.h"
#include <iostream>
#include "util/logger.h"
#include <filesystem>
#include "util/timer.h"

#include <future>

int main()
{
	//std::string path = "C:/Users/Flori/OneDrive/Documenten/GitHub/Exact-dominating-set/tests/complete_5_graph.gr";
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/testing_reduction/test_reduce_neighborhood_pair_vertex3.gr";
	bool dir_mode = true;
	std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/T1Pace_test/dat_50_50_0.gr";
	std::string dir_path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/T1Pace_test/";
	if (dir_mode) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
			reduction_info(entry.path());
		}
	}
	else {
		reduction_info(path);
	}

	return 0;
}

void reduction_info(std::string path) {
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);
	adjacencyListBoost& refGraph = adjLBoost;

	bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	Logger::is_planar = is_planar;

	//create context of the graph.
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);

	//Solve graph with ILP without reductions.
	timer t_ilp_no_reduction;
	if (operations_research::solve_dominating_set(mds_context, false)) {
		Logger::execution_ilp_without_reduction = -1; // timelimit reached.
	}
	else {
		Logger::execution_ilp_without_reduction = t_ilp_no_reduction.count();
	}

	timer t_reduction;
	reduce::log_reduce_graph(mds_context);
	Logger::execution_reduction = t_reduction.count();

	timer t_ilp_reduction;
	if (operations_research::solve_dominating_set(mds_context, true)) {
		Logger::execution_ilp_without_reduction = -1; //timelimit reached.
	}
	else {
		Logger::execution_ilp_with_reduction = t_ilp_reduction.count();
	}

	mds_context.update_vertices();
	parse::output_context(mds_context, path);

	//Log info
	std::string name = parse::getNameFile(path);
	output_loginfo(name);
}