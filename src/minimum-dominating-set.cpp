// exact-dominating-set.cpp : Defines the entry point for the application.
//
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"

#include "graph/tree_decomposition.h"
#include "reduce.h"
#include "solver.h"
#include <iostream>
#include "util/logger.h"
#include <filesystem>
#include "util/timer.h"

bool stringToBool(const std::string& str) {
	std::string s = str;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower); // Convert to lowercase

	return (s == "1" || s == "true" || s == "yes" || s == "on");
}


int main(int argc, char* argv[])
{
	//templates.
	//std::string path = "C:/Users/Flori/OneDrive/Documenten/GitHub/Exact-dominating-set/tests/complete_5_graph.gr";
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/exact_001.gr";
	//std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/pace/bremen_subgraph";

	//default values
	std::string path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/bremen_subgraph_20.gr"; 
	bool dir_mode = false;
	std::string dir_path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/";
	std::string path_td = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/tree_decomposition/reduced_instance_bremen_subgraph_20.td";

	//be able to take in parameters.
	if (argc > 1) path = std::string(argv[1]);
	if (argc > 2) dir_path = stringToBool(argv[2]);
	if (argc > 3) path_td = std::string(argv[3]);

	if (dir_mode) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
			initialize_logger();
			//temporarily disabled.
			//reduction(entry.path());
		}
	}
	else {
		reduction(path, path_td);
		//output_reduced_graph(path);
	}

	return 0;
}

void output_reduced_graph(std::string path) {
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);

	adjacencyListBoost& refGraph = adjLBoost;
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);

	reduce::reduce_ijcai(mds_context);
	//after reduction rules, define which vertices could be removed.
	mds_context.fill_removed_vertex();

	parse::output_reduced_graph_instance(mds_context, path);
}

void reduction(std::string path, std::string path_td) {
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);

	adjacencyListBoost& refGraph = adjLBoost;
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);

	timer t_reduction;
	reduce::reduce_ijcai(mds_context);
	//after reduction rules, define which vertices could be removed.
	mds_context.fill_removed_vertex();
	//reduce::refractored_reduce_graph(mds_context);
	Logger::execution_reduction = t_reduction.count();

	adjacencyListBoost reduced_graph = adjLBoost;
	auto [vert_itt, vert_itt_end] = boost::vertices(reduced_graph);
	//itterate all vertices.
	for (;vert_itt != vert_itt_end; ++vert_itt) {
		//if the vertex is either selected or excluded & dominated then remove all edges.
		if (mds_context.is_selected(*vert_itt) || (mds_context.is_dominated(*vert_itt) && mds_context.is_excluded(*vert_itt))) {
			boost::clear_vertex(*vert_itt, reduced_graph);
		}
	}

	TREE_DECOMPOSITION td_comp = parse::load_tree_decomposition(path_td, mds_context);
	timer t_treewidth;
	td_comp.create_nice_tree_decomposition(reduced_graph);
	td_comp.fill_instruction_stack();
	timer t_run_instruction;
	td_comp.run_instruction_stack();
	long long timer_2 = t_run_instruction.count();
	Logger::execution_reduction = t_treewidth.count();

	bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	Logger::is_planar = is_planar;

	//operations_research::solve_dominating_set(mds_context, true);

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

