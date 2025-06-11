// exact-dominating-set.cpp : Defines the entry point for the application.
// Made by Floris van der Hout.
#pragma once

#include "minimum-dominating-set.h"
#include "graph/graph_io.h"
#include "graph/context.h"

#include "graph/nice_tree_decomposition.h"
#include "graph/tree_decomposition.h"
#include "reduce.h"
#include <iostream>
#include "util/logger.h"
#include <filesystem>
#include <boost/graph/connected_components.hpp>

#include "sat_solver.h"
#include "graph/treewidth_solver.h"

#include "util/timer.h"
#include "graph/generate_tree_decomposition.h"

#include <thread>
#include <chrono>
#include <csignal>
#include <future>
#include <unistd.h>
#include <sched.h>

int main(int argc, char* argv[])
{
	dominating_set_solver();
	return 0;
}

void dominating_set_solver(){
	std::vector<std::unique_ptr<adjacencyListBoost>> sub_components; // subcomponents of the original problem (no reductions).
	std::vector<std::unordered_map<int, int>> sub_newToOldIndex; // translation function to get back to the original indices.
	std::vector<std::vector<std::unique_ptr<adjacencyListBoost>>> sub_sub_components; // subcomponents after reduction rules X.1 to X.3 and L.2
	std::vector<std::vector<std::unordered_map<int, int>>> sub_sub_newToOldIndex; // translation function to get back to the original indices.
	std::vector<std::vector<MDS_CONTEXT>> reduced_components_context; // AMDS context for each subcomponent.
	std::vector<std::vector<bool>> is_dominated;

	/* Takes the path, loads in the graph, and directly splits it into connected components.
	sub_components := vector with graphs.
	sub_newToOldIndex := a vector of maps which given a index of a vertex in a subcomponent translates the index
	to the original index of the vertex. */
	create_component_subgraphs(sub_components, sub_newToOldIndex);

	std::vector<int>solution; // vector which will hold all vertices in the optimal dominating set.

	// Set a timer to limit the maximum duration allowed for the reduction step.
	auto start = std::chrono::steady_clock::now();
	auto timeout_duration = std::chrono::seconds(180);

	//Handle each subcomponent separately.
	for (int i = 0; i < sub_components.size(); i++) {
		/*Try to solve the subcomponent which a minute with the SAT_solver.
		 this either provides a solution (OPTIMAL), or provides an upperbound (FEASIBlE).
		 */
		MDS_CONTEXT mds_context = MDS_CONTEXT(*sub_components[i]);

		// start reduction rule X.1 to X.3 combined with L.2 without actual removing any vertices
		strategy_reduction reduction_strategy_combination = REDUCTION_COMBINATION;
		reduce::reduction_rule_manager(mds_context, reduction_strategy_combination, 0, false, start, timeout_duration);

		mds_context.fill_removed_vertex();

		// Fill the solution with vertices which must be in the dominating set.
		for (int v = 0; v < mds_context.selected.size(); ++v) {
			if (mds_context.is_selected(v)) {
				// Adjust by +1 to convert from 0-indexed to 1-indexed vertices.
				solution.push_back(sub_newToOldIndex[i][v] + 1);
			}
		}

		//The previous reductions could split up problems into smaller sub-problems.
		sub_sub_components.emplace_back();
		sub_sub_newToOldIndex.emplace_back();
		is_dominated.emplace_back();

		std::unordered_map<int, int> newToOldIndex;
		adjacencyListBoost reduced_graph = create_reduced_graph(mds_context, newToOldIndex);
		create_reduced_component_subgraphs(reduced_graph, sub_sub_components[i], sub_sub_newToOldIndex[i], newToOldIndex);

		reduced_components_context.emplace_back();

		for (int j = 0; j < sub_sub_components[i].size(); ++j) {
			reduced_components_context[i].emplace_back(*sub_sub_components[i][j]);
			reduced_components_context[i][j].fill_mds_context(mds_context, sub_sub_newToOldIndex[i][j]);
			is_dominated[i].emplace_back(false);
		}
	}
	// After removing all omittable vertices reduction rule L.3 to L.5 can be applied.
	strategy_reduction reduction_strategy_l = REDUCTION_L_ALBER;
	//Ensure uniform rule application across all subcomponents first, to avoid uneven computational load on individual components.
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - start;
    auto time_left = timeout_duration - elapsed;
    if (elapsed * 50 < time_left){
	for (int i = 0; i < sub_components.size(); ++i) {
		int rule_id = 3;
		for (int j = 0; j < sub_sub_components[i].size(); ++j) {
			//no further reductions possible.
			if (reduced_components_context[i][j].num_undetermined_vertices() == 0 || is_dominated[i][j]){
					continue;
			}
				reduce::reduction_rule_manager(reduced_components_context[i][j], reduction_strategy_l, rule_id, false, start, timeout_duration);
		}
	}
}
    auto now2 = std::chrono::steady_clock::now();
    elapsed = now2 - start;
    time_left = timeout_duration - elapsed;
    if (elapsed * 50 < time_left){
	for (int i = 0; i < sub_components.size(); ++i) {
		int rule_id = 4;
		for (int j = 0; j < sub_sub_components[i].size(); ++j) {
			//no further reductions possible.
			if (reduced_components_context[i][j].num_undetermined_vertices() == 0 || is_dominated[i][j]){
				continue;
			}
			reduce::reduction_rule_manager(reduced_components_context[i][j], reduction_strategy_l, rule_id, false, start, timeout_duration);
		}
	}
}
    auto now3 = std::chrono::steady_clock::now();
    elapsed = now3 - start;
    time_left = timeout_duration - elapsed;
    if (elapsed * 50 < time_left){
	for (int i = 0; i < sub_components.size(); ++i) {
		int rule_id = 5;
		for (int j = 0; j < sub_sub_components[i].size(); ++j) {
			//no further reductions possible.
			if (reduced_components_context[i][j].num_undetermined_vertices() == 0 || is_dominated[i][j]){
				continue;
			}
			reduce::reduction_rule_manager(reduced_components_context[i][j], reduction_strategy_l, rule_id, false, start, timeout_duration);
		}
	}
}
	// Fill the solution with vertices which must be in the dominating set because of the reduction rules.

	for (int i = 0; i < sub_components.size(); ++i){
		for (int j = 0; j < sub_sub_components[i].size(); ++j)
		{
			if (is_dominated[i][j]){
				continue;
			}
			reduced_components_context[i][j].fill_removed_vertex();
			for (int v = 0; v < reduced_components_context[i][j].selected.size(); ++v){
				//we need a +1 te correct the previous -1.
				if (reduced_components_context[i][j].is_selected(v)){
					solution.push_back(((sub_newToOldIndex[i][sub_sub_newToOldIndex[i][j][v]]) + 1));
				}
			}
			//These reduction rules can cause components to subdivide into subcomponents so split them.
			std::vector<std::unique_ptr<adjacencyListBoost>> sub_sub_sub_components;
			std::vector<std::unordered_map<int, int>> sub_sub_sub_newToOldIndex;

			std::unordered_map<int, int> newToOld;
			adjacencyListBoost reduced_graph = create_reduced_graph(reduced_components_context[i][j], newToOld);
			create_reduced_component_subgraphs(reduced_graph, sub_sub_sub_components, sub_sub_sub_newToOldIndex, newToOld);

			for (int q = 0; q < sub_sub_sub_components.size(); ++q) {
				//fast check if it can be solved easily.
				 if (boost::num_vertices(*sub_sub_sub_components[q]) < 5000)
				{
					std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition = generate_td(*sub_sub_sub_components[q]);
					if (nice_tree_decomposition == nullptr) {

					} else {
						if (nice_tree_decomposition->treewidth <= 14){
							std::unique_ptr<TREEWIDTH_SOLVER> td_comp = std::make_unique<TREEWIDTH_SOLVER>(std::move(nice_tree_decomposition), reduced_components_context[i][j].dominated, reduced_components_context[i][j].excluded, sub_sub_sub_newToOldIndex[q]);

							//generate final solution.
							for (int newIndex : td_comp->global_solution) {
								auto sub_index = sub_sub_newToOldIndex[i][j][sub_sub_sub_newToOldIndex[q][newIndex]];
								solution.push_back((sub_newToOldIndex[i][sub_index]) + 1);
							}
							continue;
						}
					}
				}
				//If treewidth to big? -> SAT solver.
				std::vector<int> partial_solution = sat_solver_dominating_set(reduced_components_context[i][j], *sub_sub_sub_components[q], sub_sub_sub_newToOldIndex[q]);
				for (int newIndex : partial_solution) {
					auto sub_index = sub_sub_newToOldIndex[i][j][sub_sub_sub_newToOldIndex[q][newIndex]];
					solution.push_back((sub_newToOldIndex[i][sub_index]) + 1);
				}
			}
		}
	}
	parse::print_solution(solution);
}

void create_reduced_component_subgraphs(adjacencyListBoost& reduced_graph,
										std::vector<std::unique_ptr<adjacencyListBoost>>& sub_sub_components,
										std::vector<std::unordered_map<int, int>>& sub_sub_newToOldIndex,
										std::unordered_map<int, int>& newToOldIndex){

	std::vector<int> component_map(boost::num_vertices(reduced_graph));
	int num_components = boost::connected_components(reduced_graph, &component_map[0]);
	sub_sub_components.resize(num_components);
	sub_sub_newToOldIndex.resize(num_components);
	Logger::num_reduced_components += num_components;

	if (num_components == 1){
		sub_sub_components[0] = std::make_unique<adjacencyListBoost>(reduced_graph);
		sub_sub_newToOldIndex[0] = newToOldIndex;
		return;
	}

	std::vector<std::vector<int>> components(num_components);

	//distribute components.
	for (size_t i = 0; i < component_map.size(); ++i) {
		components[component_map[i]].push_back(i);
	}

	//Create empty sub graphs.
	for (size_t i = 0; i < components.size(); ++i) {
		std::unique_ptr<adjacencyListBoost> sub_sub_component = std::make_unique<adjacencyListBoost>(components[i].size());
		sub_sub_components[i] = std::move(sub_sub_component);
	}

	//translate function.
	for (size_t i = 0; i < boost::num_vertices(reduced_graph); ++i) {
		int component_i = component_map[i];
		auto it_i = std::find(components[component_i].begin(), components[component_i].end(), i);
		auto index_i = std::distance(components[component_i].begin(), it_i);
		sub_sub_newToOldIndex[component_i].insert({index_i,newToOldIndex[i]});
	}

	//add all edges.
	for (auto edge_iter = edges(reduced_graph); edge_iter.first != edge_iter.second; ++edge_iter.first) {
		auto u = source(*edge_iter.first, reduced_graph);
		auto v = target(*edge_iter.first, reduced_graph);
		int component_u = component_map[u];
		int component_v = component_map[v];

		if (component_u == component_v) {
			auto it_u = std::find(components[component_u].begin(), components[component_u].end(), u);
			auto it_v = std::find(components[component_v].begin(), components[component_v].end(), v);
			auto index_u = std::distance(components[component_u].begin(), it_u);
			auto index_v = std::distance(components[component_v].begin(), it_v);
			boost::add_edge(index_u, index_v, *sub_sub_components[component_u]);
			//sub_sub_newToOldIndex[component_u].insert({index_u,newToOldIndex[u]});
			//sub_sub_newToOldIndex[component_v].insert({index_v,newToOldIndex[v]});
		} else {
			throw std::runtime_error("Edge endpoints must always be in the same component.");
		}
	}


}

void split_graph_component(adjacencyListBoost& graph ,std::vector<std::unique_ptr<adjacencyListBoost>>& sub_components,
								std::vector<std::unordered_map<int, int>>& sub_newToOldIndex) {

}

void create_component_subgraphs(std::vector<std::unique_ptr<adjacencyListBoost>>& sub_components,
								std::vector<std::unordered_map<int, int>>& sub_newToOldIndex){

	std::unique_ptr<adjacencyListBoost> graph = std::make_unique<adjacencyListBoost>(parse::read_pace_2024(std::cin));
	Logger::num_vertices = num_vertices(*graph);
	Logger::num_edges = num_edges(*graph);

	//Find components (which can be solved separately).
	std::vector<int> component_map(boost::num_vertices(*graph));
	int num_components = boost::connected_components(*graph, &component_map[0]);
	Logger::num_components = num_components;

	sub_components.resize(num_components);
	sub_newToOldIndex.resize(num_components);

	//no different components.
	if (num_components == 1){

		for (int q = 0; q < boost::num_vertices(*graph); q++) {
			sub_newToOldIndex[0].insert({q, q});
		}
		sub_components[0] = std::move(graph);
		return;
	}

	std::vector<std::vector<int>> components(num_components);

	//distribute components.
	for (size_t i = 0; i < component_map.size(); ++i) {
		components[component_map[i]].push_back(i);
	}

	//Create empty sub graphs.
	for (size_t i = 0; i < components.size(); ++i) {
		std::unique_ptr<adjacencyListBoost> sub_component = std::make_unique<adjacencyListBoost>(components[i].size());
		sub_components[i] = std::move(sub_component);
	}

	//translate function.
	for (size_t i = 0; i < boost::num_vertices(*graph); ++i) {
		int component_i = component_map[i];
		auto it_i = std::find(components[component_i].begin(), components[component_i].end(), i);
		auto index_i = std::distance(components[component_i].begin(), it_i);
		sub_newToOldIndex[component_i].insert({index_i,i});
	}

	//add all edges.
	for (auto edge_iter = edges(*graph); edge_iter.first != edge_iter.second; ++edge_iter.first) {
		auto u = source(*edge_iter.first, *graph);
		auto v = target(*edge_iter.first, *graph);
		int component_u = component_map[u];
		int component_v = component_map[v];

		if (component_u == component_v) {
			auto it_u = std::find(components[component_u].begin(), components[component_u].end(), u);
			auto it_v = std::find(components[component_v].begin(), components[component_v].end(), v);
			auto index_u = std::distance(components[component_u].begin(), it_u);
			auto index_v = std::distance(components[component_v].begin(), it_v);
			boost::add_edge(index_u, index_v, *sub_components[component_u]);
			//sub_newToOldIndex[component_u].insert({index_u,u});
			//sub_newToOldIndex[component_v].insert({index_v,v});
		} else {
			throw std::runtime_error("Edge endpoints must always be in the same component.");
		}
	}
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

		//only add edges which both exists in the new graph. !(mds_context.is_dominated(source) & mds_context.is_dominated(target))
		if (OldToNewIndex.count(source) && OldToNewIndex.count(target)) {
			// if (mds_context.is_excluded(source) && mds_context.is_excluded(target)) {
			//
			// } else
			// {
			boost::add_edge(OldToNewIndex[source], OldToNewIndex[target], reduced_graph);
			// }
		}
	}
	return reduced_graph;
}
	

