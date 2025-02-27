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

int main()
{
	//std::string path = "C:/Users/Flori/OneDrive/Documenten/GitHub/Exact-dominating-set/tests/complete_5_graph.gr";
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/exact_001.gr";
	std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/exact_017.gr";
	bool dir_mode = false;
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/pace/bremen_subgraph";
	std::string dir_path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/";
	if (dir_mode) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
			initialize_logger();
			reduction(entry.path());
		}
	}
	else {
		reduction(path);
	}

	return 0;
}

void reduction(std::string path) {
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);
	adjacencyListBoost& refGraph = adjLBoost;

	bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	Logger::is_planar = is_planar;

	//create context of the graph.
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);

	//operations_research::solve_dominating_set(mds_context, true);

	timer t_reduction;
	reduce::reduce_ijcai(mds_context);
	//reduce::refractored_reduce_graph(mds_context);
	Logger::execution_reduction = t_reduction.count();

	timer t_ilp_reduction;
	//if (operations_research::solve_dominating_set(mds_context, true)) {
	//	Logger::execution_ilp_without_reduction = -1; //timelimit reached.
	//}
	//else {
	//	Logger::execution_ilp_with_reduction = t_ilp_reduction.count();
	//}

	parse::output_context(mds_context, path);

	//get reduction results
	std::vector<int>dominated;
	for (int i = 0; i < mds_context.get_total_vertices(); ++i) {
		if (mds_context.dominated[i] == 1) {
			dominated.push_back(i);
		}
	}
	std::vector<int>selected;
	for (int i = 0; i < mds_context.get_total_vertices(); ++i) {
		if (mds_context.selected[i] == 1) {
			selected.push_back(i);
		}
	}
	std::vector<int>removed;
	for (int i = 0; i < mds_context.get_total_vertices(); ++i) {
		if (mds_context.removed[i] == 1) {
			removed.push_back(i);
		}
	}
	std::vector<int>ignored;
	for (int i = 0; i < mds_context.get_total_vertices(); ++i) {
		if (mds_context.ignored[i] == 1) {
			ignored.push_back(i);
		}
	}
	std::vector<int>excluded;
	for (int i = 0; i < mds_context.get_total_vertices(); ++i) {
		if (mds_context.excluded[i] == 1) {
			excluded.push_back(i);
		}
	}

	//Log info
	std::string name = parse::getNameFile(path);
	output_loginfo(name, selected, dominated, removed, ignored, excluded);
	parse::output_context(mds_context, path);
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
	//reduce::log_reduce_graph(mds_context);
	Logger::execution_reduction = t_reduction.count();

	timer t_ilp_reduction;
	if (operations_research::solve_dominating_set(mds_context, true)) {
		Logger::execution_ilp_without_reduction = -1; //timelimit reached.
	}
	else {
		Logger::execution_ilp_with_reduction = t_ilp_reduction.count();
	}

	parse::output_context(mds_context, path);

	//Log info
	std::string name = parse::getNameFile(path);
	//output_loginfo(name); TODO::DONT FORGET TO RETURN
}