// exact-dominating-set.cpp : Defines the entry point for the application.
// Made by Floris van der Hout.
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"

#include "graph/nice_tree_decomposition.h"
#include "graph/tree_decomposition.h"
#include "reduce.h"
#include "solver.h"
#include <iostream>
#include "util/logger.h"
#include <filesystem>
#include <boost/graph/connected_components.hpp>

#include "sat_solver.h"
#include "graph/treewidth_solver.h"

#include "util/timer.h"
#include "graph/generate_tree_decomposition.h"
#include "ortools/sat/cp_model_solver.h"

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
	std::string path = "/home/floris/Documents/Thesis/Dataset/Exact/exact_001.gr"; //original graph.
	bool dir_mode = false;
	std::string dir_path = "/mnt/c/Users/Flori/OneDrive/Universiteit-Utrecht/Thesis/code/parser/dataset/exact/";
	std::string path_td = "/home/floris/Documents/Thesis/Dataset/Tree_decomposition/reduced_instance_exact_001.txt"; //

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
		//component_reduction(path);
		//reduction(path, path_td);
		output_reduced_graph(path);
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

	timer t_sat;
	ds_sat_solver(mds_context);
	std::cout << t_sat.count() << "\n";
	//remove edges which are not needed (we do this because we only want to introduce the needed vertices (for the reduced graph).
	std::unordered_map<int, int> newToOldIndex;
	adjacencyListBoost reduced_graph = create_reduced_graph(mds_context, newToOldIndex);

	parse::output_reduced_graph_instance(reduced_graph, path);
}

void component_reduction(std::string path)
{
	timer t_complete;
	//create the original graph & context.
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);

	//run reduction rules & fill the context.
	MDS_CONTEXT mds_context = MDS_CONTEXT(adjLBoost);
	reduce::reduce_ijcai(mds_context);
	mds_context.fill_removed_vertex();

	//Remove the unneeded vertices. (the reduced graph is 0 indexed so we have a map from new indicies and the old ones).
	std::unordered_map<int, int> newToOldIndex;
	adjacencyListBoost reduced_graph = create_reduced_graph(mds_context, newToOldIndex);


	std::vector<int> component_map_reduced(boost::num_vertices(reduced_graph));
	std::vector<int> component_map_default(boost::num_vertices(adjLBoost));
	int num_components_reduced = boost::connected_components(reduced_graph, &component_map_reduced[0]);
	int num_components_default = boost::connected_components(adjLBoost, &component_map_default[0]);

	std::vector<std::vector<int>> components(num_components_reduced);

	//distribute components.
	for (size_t i = 0; i < component_map_reduced.size(); ++i) {
		components[component_map_reduced[i]].push_back(i);
	}

	std::cout << num_components_reduced << std::endl;
	std::cout << num_components_default << std::endl;

	//create empty sub-graphs + translation function.
	std::vector<std::unique_ptr<adjacencyListBoost>> sub_components(num_components_reduced);
	std::vector<std::unordered_map<int, int>> sub_newToOldIndex(num_components_reduced);

	for (size_t i = 0; i < components.size(); ++i) {
		std::unique_ptr<adjacencyListBoost> sub_component = std::make_unique<adjacencyListBoost>(components[i].size());
		sub_components[i] = std::move(sub_component);
	}

	//add all edges.
	for (auto edge_iter = edges(reduced_graph); edge_iter.first != edge_iter.second; ++edge_iter.first) {
		auto u = source(*edge_iter.first, reduced_graph);
		auto v = target(*edge_iter.first, reduced_graph);
		int component_u = component_map_reduced[u];
		int component_v = component_map_reduced[v];

		if (component_u == component_v) {
			auto it_u = std::find(components[component_u].begin(), components[component_u].end(), u);
			auto it_v = std::find(components[component_v].begin(), components[component_v].end(), v);
			auto index_u = std::distance(components[component_u].begin(), it_u);
			auto index_v = std::distance(components[component_v].begin(), it_v);
			boost::add_edge(index_u, index_v, *sub_components[component_u]);
			sub_newToOldIndex[component_u].insert({index_u,newToOldIndex[u]});
			sub_newToOldIndex[component_v].insert({index_v,newToOldIndex[v]});
		} else {
			throw std::runtime_error("Edge endpoints must always be in the same component.");
		}
	}
	std::vector<int>solution;
	for (int i = 0; i < sub_components.size(); ++i){
		std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition = generate_td(*sub_components[i]);
		std::unique_ptr<TREEWIDTH_SOLVER> td_comp = std::make_unique<TREEWIDTH_SOLVER>(std::move(nice_tree_decomposition), mds_context.dominated, mds_context.excluded, sub_newToOldIndex[i]);


		// std::unique_ptr<TREE_DECOMPOSITION> td_comp = std::make_unique<TREE_DECOMPOSITION>(std::move(nice_tree_decomposition));
		// td_comp->fill_instruction_stack();
		// td_comp->run_instruction_stack(mds_context.dominated, mds_context.excluded, sub_newToOldIndex[i]);

		//generate final solution.
		for (int newIndex : td_comp->global_solution) {
			//we need a +1 te correct the previous -1.
			solution.push_back((sub_newToOldIndex[i][newIndex]) + 1);
		}
	}

	for (int i = 0; i < mds_context.selected.size(); ++i) {
		if (mds_context.is_selected(i)) {
			//we need a +1 te correct the previous -1.
			solution.push_back(i + 1);
		}
	}
	//return domination number.
	std::cout << "execution_time_join: "<< Logger::execution_time_join << std::endl;
	std::cout << "execution_time_Introduce: "<< Logger::execution_time_introduce << std::endl;
	std::cout << "execution_time_Introduce_edge: "<< Logger::execution_time_introduce_edge << std::endl;
	std::cout << "execution_time_forget: "<< Logger::execution_time_forget << std::endl;
	std::cout << "execution_time_leaf: "<< Logger::execution_time_leaf << std::endl;
	std::cout << t_complete.count() << std::endl;
	std::cout << solution.size() << std::endl;
	std::sort(solution.begin(), solution.end());
	parse::output_solution(solution, path);
}





void reduction(std::string path, std::string path_td) {
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);

	adjacencyListBoost& refGraph = adjLBoost;
	MDS_CONTEXT mds_context = MDS_CONTEXT(refGraph);

	timer t_reduction;
	reduce::reduce_ijcai(mds_context);
	//reduce::refractored_reduce_graph(mds_context);
	//after reduction rules, define which vertices could be removed.
	mds_context.fill_removed_vertex();

	Logger::execution_reduction = t_reduction.count();

	//remove edges which are not needed (we do this because we only want to introduce the needed vertices (for the reduced graph).
	std::unordered_map<int, int> newToOldIndex;
	std::unordered_map<int, int> oldToNewIndex;
	adjacencyListBoost reduced_graph = create_reduced_graph(mds_context, newToOldIndex);

	std::vector<int> component_map(boost::num_vertices(reduced_graph));
	int num_components = boost::connected_components(reduced_graph, &component_map[0]);

	std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition = generate_td(reduced_graph);
	//std::unique_ptr<TREE_DECOMPOSITION> td_comp = std::make_unique<TREE_DECOMPOSITION>(std::move(nice_tree_decomposition));

	//td_comp->fill_instruction_stack();
	//td_comp->run_instruction_stack(mds_context.dominated, mds_context.excluded, newToOldIndex);

	// timer t_treewidth;
	// td_comp.create_nice_tree_decomposition(reduced_graph);
	// td_comp.fill_instruction_stack();
	// timer t_run_instruction;
	// td_comp.run_instruction_stack(mds_context.dominated, newToOldIndex);
	//
	// //create a solution.
	// std::vector<int>solution;
	// int domination_number = mds_context.cnt_sel + td_comp->global_solution.size();
	// solution.reserve(domination_number);
	// //get vertices old index from global solution (which uses the new index)
	// for (int newIndex : td_comp->global_solution) {
	// 	//we need a +1 te correct the previous -1.
	// 	if (mds_context.is_excluded(newToOldIndex[newIndex]))
	// 	{
	// 		throw std::runtime_error("should not be allowed");
	// 	}
	// 	solution.push_back(newToOldIndex[newIndex] + 1);
	// }
	// for (int i = 0; i < mds_context.selected.size(); ++i) {
	// 	if (mds_context.is_selected(i)) {
	// 		//we need a +1 te correct the previous -1.
	// 		solution.push_back(i + 1);
	// 	}
	// }
	// //
	// // long long timer_2 = t_run_instruction.count();
	// // Logger::execution_reduction = t_treewidth.count();
	// // std::cout << solution.size() << std::endl;
	// // std::cout << t_treewidth.count() << std::endl;
	//
	// std::cout << solution.size() << std::endl;
	// bool is_planar = boost::boyer_myrvold_planarity_test(adjLBoost);
	// Logger::is_planar = is_planar;

	//operations_research::solve_dominating_set(mds_context, true);

	timer t_ilp_reduction;
	//if (operations_research::solve_dominating_set(mds_context, true)) {
	//	Logger::execution_ilp_without_reduction = -1; //timelimit reached.
	//}
	//else {
	//	Logger::execution_ilp_with_reduction = t_ilp_reduction.count();
	//}

	//parse::output_solution(solution , path);

	//parse::output_context(mds_context, path);

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
	//std::string name = parse::getNameFile(path);
	//output_loginfo(name, selected, dominated, removed, ignored, excluded);
	//parse::output_context(mds_context, path);
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

adjacencyListBoost create_reduced_graph(MDS_CONTEXT& mds_context, std::unordered_map<int, int>& newToOldIndex) {

	std::unordered_map<int, int> OldToNewIndex;

	auto [vert_itt, vert_itt_end] = boost::vertices(mds_context.graph);

	//fill newToOldIndex, this way we can find the original indexes after.
	int newIndex = 0;
	for (int oldIndex = 0; oldIndex < boost::num_vertices(mds_context.graph); ++oldIndex) {
		if (!mds_context.is_selected(oldIndex) && !mds_context.is_removed(oldIndex)) {
			newToOldIndex[newIndex] = oldIndex;
			OldToNewIndex[oldIndex] = newIndex;
			++newIndex;
		}
	}

	//create the new graph with the updated number of vertices
	adjacencyListBoost reduced_graph(OldToNewIndex.size());

	for (auto edge : make_iterator_range(mds_context.get_edge_itt())) {
		int source = boost::source(edge, mds_context.graph);
		int target = boost::target(edge, mds_context.graph);

		//only add edges which both exists in the new graph.
		if (OldToNewIndex.count(source) && OldToNewIndex.count(target)) {
			boost::add_edge(OldToNewIndex[source], OldToNewIndex[target], reduced_graph);
		}
	}
	return reduced_graph;
}

	

