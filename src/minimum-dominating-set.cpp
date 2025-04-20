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

#include <thread>
#include <chrono>
#include <csignal>
#include <future>
#include <unistd.h>

bool stringToBool(const std::string& str) {
	std::string s = str;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower); // Convert to lowercase

	return (s == "1" || s == "true" || s == "yes" || s == "on");
}

strategy_solver string_to_strategy_solver(const std::string& str) {
	if (str == "sat" || str == "SAT") {
		return SOLVER_SAT;
	}
	if (str == "ilp" || str == "ILP") {
		return SOLVER_ILP;
	}
	if (str == "treewidth" || str == "TREEWIDTH"){
		return SOLVER_TREEWIDTH;
	}
	if (str == "combination" || str == "COMBINATION") {
		return SOLVER_COMBINATION; // TODO:: implement.
	}
	if (str == "non" || str == "none") {
		return SOLVER_NON;
	}
	if (str == "nice_tree_decomposition") {
		return SOLVER_NICE_TREE_DECOMPOSITION;
	}
	throw new std::runtime_error("not a viable strategy");

}

strategy_reduction string_to_strategy_reduction(const std::string& str) {
	if (str == "none" || str == "non") {
		return REDUCTION_NON;
	}
	if (str == "alber" || str == "ALBER") {
		return REDUCTION_ALBER;
	}
	if (str == "alber_rule_1" || str == "ALBER_RULE_1") {
		return REDUCTION_ALBER_RULE_1;
	}
	if (str == "IJCAI" || str == "ijcai") {
		return REDUCTION_IJCAI;
	}
	if (str == "COMBINATION" || str == "combination") {
		return REDUCTION_COMBINATION;
	}
	throw new std::runtime_error("not a viable strategy");
}

void signal_handler(int signum) {
	std::cout << "Received signal " << signum << ". Stopping the main task...\n";
	//print temporary results.
}

void timer_thread(std::future<void>& main_future){
	std::cout << "Timer thread waiting for 30 minutes...\n";

	// Wait for either the main task to finish or 30 minutes to pass
	if (main_future.wait_for(std::chrono::minutes(30)) == std::future_status::timeout) {
		// Timeout reached (30 minutes passed) and main task is still running
		std::cout << "30 minutes passed. Sending SIGINT to stop main task...\n";
		kill(getpid(), SIGINT);  // Send SIGINT to the current process
	} else {
		// Main task finished before the timeout
		std::cout << "Main task completed early. Timer thread exiting...\n";
	}
}

int main(int argc, char* argv[])
{

	//default values
	// path : string with path to instance graph.
	bool dir_mode = true;
	std::string dir_path = "/home/floris/Documents/Thesis/Dataset/Exact/";
	std::string path = "/home/floris/Documents/Thesis/Dataset/Exact/exact_033.gr";
	//reduction_strategy: [options: Alber, Alber_rule_1, IJCAI, Combination, non]
	strategy_reduction reduction_strategy = REDUCTION_COMBINATION;
	//Solver_strategy: [options: ILP, SAT, Treewidth, Combination, non]
	strategy_solver solver_strategy = SOLVER_NON;
	strategy_reduction_scheme reduction_scheme_strategy = REDUCTION_ALBER_L_3;


	//be able to take in parameters.
	if (argc > 1) path = std::string(argv[1]);
	if (argc > 2) reduction_strategy = string_to_strategy_reduction(std::string(argv[2]));
	if (argc > 3) solver_strategy = string_to_strategy_solver(std::string(argv[3]));

	signal(SIGINT, signal_handler);
	//Sigint handler.
	if (dir_mode) {
		for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
			std::cout << entry.path().string() << std::endl;
			initialize_logger();
			separate_solver(entry.path(), reduction_strategy, solver_strategy, reduction_scheme_strategy);
		}
	} else {
		separate_solver(path, reduction_strategy, solver_strategy, reduction_scheme_strategy);
		//seperate_solver_no_components(path, reduction_strategy, solver_strategy);
	}

	// std::promise<void> main_promise;
	// std::future<void> main_future = main_promise.get_future();
	//
	// std::thread main_thread([&main_promise, &path, &reduction_strategy,&solver_strategy]() {
	//  	separate_solver(path, reduction_strategy, solver_strategy);
	//  	main_promise.set_value();  // Notify that the main task is finished
	// });

	// Create and launch the timer thread
	//std::thread timer(timer_thread, std::ref(main_future));

	// Wait for the main task thread to finish (either by completion or signal)
	//main_thread.join();
	// Wait for the timer thread to finish (it may finish early if main task completes)
	//timer.join();
	return 0;
}

void separate_solver(std::string path, strategy_reduction red_strategy, strategy_solver sol_strategy, strategy_reduction_scheme red_scheme_strategy){
	timer t_complete;
	Logger::solver_strategy = sol_strategy;
	Logger::reduction_strategy = red_strategy;
	Logger::reduction_scheme_strategy = red_scheme_strategy;
	//create empty sub-graphs + translation function.
	std::vector<std::unique_ptr<adjacencyListBoost>> sub_components;
	std::vector<std::unordered_map<int, int>> sub_newToOldIndex;

	//Fill sub-graphs + translation function (no reduction).
	create_component_subgraphs(path, sub_components, sub_newToOldIndex);

	std::vector<int>solution;

	for (int i = 0; i < sub_components.size(); ++i){
		//Create a mds_context & reduce. for each subgraph.
		std::atomic<bool> stop_flag(false);
		MDS_CONTEXT mds_context = MDS_CONTEXT(*sub_components[i]);
		timer t_reduction;
		std::cout << "start reduction:" << std::endl;
		std::future<void> reduction = std::async(std::launch::async, reduce::reduction_rule_manager, std::ref(mds_context), std::ref(red_strategy), 0, std::ref(stop_flag));

		if (reduction.wait_for(std::chrono::minutes(20)) == std::future_status::ready){
			reduction.get();
		} else{
			stop_flag = true;
		}

		Logger::execution_time_reduction += t_reduction.count();
		mds_context.fill_removed_vertex();

		Logger::cnt_selected_vertices += mds_context.cnt_sel;
		//Logger::cnt_excluded_vertices += mds_context.cnt_excl;
		//Logger::cnt_ignored_vertices += mds_context.cnt_ign;
		Logger::cnt_removed_vertices += mds_context.cnt_rem;

		for (int v = 0; v < mds_context.selected.size(); ++v) {
			if (mds_context.is_selected(v)) {
				//we need a +1 te correct the previous -1.
				solution.push_back(sub_newToOldIndex[i][v] + 1);
			}
		}

		//Has reduction broke up a subgraph, into multiple sub-graphs.
		std::vector<std::unique_ptr<adjacencyListBoost>> sub_sub_components;
		std::vector<std::unordered_map<int, int>> sub_sub_newToOldIndex;

		std::unordered_map<int, int> newToOldIndex;
		adjacencyListBoost reduced_graph = create_reduced_graph(mds_context, newToOldIndex);
		create_reduced_component_subgraphs(reduced_graph, sub_sub_components, sub_sub_newToOldIndex, newToOldIndex);
		//Solve each subgraph with a solver.
		for (int j = 0; j < sub_sub_components.size(); ++j)
		{

			//get new MDS_CONTEXT.
			MDS_CONTEXT mds_context_reduced = MDS_CONTEXT(*sub_sub_components[j]);
			mds_context_reduced.fill_mds_context(mds_context, sub_sub_newToOldIndex[j]);

			strategy_reduction strategy = REDUCTION_L_ALBER;
			// std::future<void> l_reduction = std::async(std::launch::async, reduce::reduction_rule_manager, std::ref(mds_context_reduced), std::ref(strategy), 4, std::ref(stop_flag));
			//
			// if (l_reduction.wait_for(std::chrono::minutes(10)) == std::future_status::ready){
			// 	l_reduction.get();
			// } else{
			// 	stop_flag = true;
			// }
			timer t_reduction_alber_rule_l;
			if (!(red_scheme_strategy == REDUCTION_ALBER_L_NON)) {
				if (red_scheme_strategy == REDUCTION_ALBER_L_3) {
					std::future<void> l_reduction = std::async(std::launch::async, reduce::reduction_rule_manager, std::ref(mds_context_reduced), std::ref(strategy), 3, std::ref(stop_flag));
					if (l_reduction.wait_for(std::chrono::minutes(30)) == std::future_status::ready){
							l_reduction.get();
						} else{
							stop_flag = true;
							Logger::timed_out = true;
						}
				} else {
					reduce::reduction_rule_manager(mds_context_reduced, strategy, 3, stop_flag);
					reduce::reduction_rule_manager(mds_context_reduced, strategy, 4, stop_flag);

				}

			}
			Logger::execution_time_alber_rule_l += t_reduction_alber_rule_l.count();
			Logger::execution_time_reduction += t_reduction_alber_rule_l.count();

			mds_context_reduced.fill_removed_vertex();
			//Logger::cnt_selected_vertices += mds_context_reduced.cnt_sel;
			//Logger::cnt_excluded_vertices += mds_context_reduced.cnt_excl;
			//Logger::cnt_ignored_vertices += mds_context_reduced.cnt_ign;
			Logger::cnt_removed_vertices += mds_context_reduced.cnt_rem;

			std::unordered_map<int, int> newToOld;
			for (int v = 0; v < mds_context_reduced.selected.size(); ++v) {
				if (mds_context_reduced.is_selected(v)) {
					//we need a +1 te correct the previous -1.
					solution.push_back(((sub_newToOldIndex[i][sub_sub_newToOldIndex[j][v]]) + 1));
					Logger::cnt_selected_vertices++;
					continue;
				}
				if (mds_context_reduced.is_removed(v)) {
					continue;
				}
				Logger::num_reduced_vertices++;
				if (mds_context_reduced.is_undetermined(v)){
					Logger::cnt_undetermined_vertices++;
				}
				if (mds_context_reduced.is_dominated(v))
				{
					Logger::cnt_dominated_vertices++;
				}
				if (mds_context_reduced.is_excluded(v))
				{
					Logger::cnt_excluded_vertices++;
				}
				if (mds_context_reduced.is_ignored(v)){
					Logger::cnt_ignored_vertices++;
				}
			}
			adjacencyListBoost more_reduced = create_reduced_graph(mds_context_reduced, newToOld);



			if (sol_strategy == SOLVER_NICE_TREE_DECOMPOSITION){
				timer t_nice_tree_decomposition;
				std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition = generate_td(more_reduced);
				if (nice_tree_decomposition)
				{
					if (Logger::maximum_treewidth < nice_tree_decomposition->treewidth){
						Logger::maximum_treewidth = nice_tree_decomposition->treewidth;
					}
				}
				Logger::execution_time_nice_tree_decomposition += t_nice_tree_decomposition.count();
			}
			if (sol_strategy == SOLVER_TREEWIDTH)
			{
				timer t_treewidth;
				std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition = generate_td(more_reduced);
				if (nice_tree_decomposition == nullptr)
				{
					throw std::runtime_error("tree decomposition is to big");
				}
				std::unique_ptr<TREEWIDTH_SOLVER> td_comp = std::make_unique<TREEWIDTH_SOLVER>(std::move(nice_tree_decomposition), mds_context.dominated, mds_context.excluded, sub_sub_newToOldIndex[j]);

				//generate final solution.
				for (int newIndex : td_comp->global_solution) {
					auto sub_index = sub_sub_newToOldIndex[j][newToOld[newIndex]];
					solution.push_back((sub_newToOldIndex[i][sub_index]) + 1);
				}
				Logger::execution_time_treewidth += t_treewidth.count();
			}
			else if (sol_strategy == SOLVER_ILP) {
				timer t_ilp;
				std::vector<int> partial_solution = operations_research::ilp_solver(mds_context_reduced, more_reduced, newToOld);

				for (int newIndex : partial_solution) {
					auto sub_index = sub_sub_newToOldIndex[j][newToOld[newIndex]];
					solution.push_back((sub_newToOldIndex[i][sub_index]) + 1);
				}
				Logger::execution_time_ilp += t_ilp.count();
			}
			else if (sol_strategy == SOLVER_SAT) {
				timer t_sat;
				std::vector<int> partial_solution = sat_solver_dominating_set(mds_context_reduced, more_reduced, newToOld);

				for (int newIndex : partial_solution) {
					auto sub_index = sub_sub_newToOldIndex[j][newToOld[newIndex]];
					solution.push_back((sub_newToOldIndex[i][sub_index]) + 1);
				}
				Logger::execution_time_sat += t_sat.count();
			}
		}
	}
	Logger::domination_number = solution.size();
	Logger::execution_time_complete = t_complete.count();
	std::cout << Logger::execution_time_complete << std::endl;
	parse::output_solution(solution, path);
	std::string name = parse::getNameFile(path);
	output_loginfo(name);
}

void seperate_solver_no_components(std::string path, strategy_reduction red_strategy, strategy_solver sol_strategy){
	timer t_complete;
	Logger::solver_strategy = sol_strategy;
	Logger::reduction_strategy = red_strategy;

	std::unique_ptr<adjacencyListBoost> graph = std::make_unique<adjacencyListBoost>(parse::load_pace_2024(path));
	MDS_CONTEXT mds_context = MDS_CONTEXT(*graph);
	timer t_reduction;
	//reduce::reduction_rule_manager(mds_context, red_strategy);
	Logger::execution_time_reduction += t_reduction.count();
	mds_context.fill_removed_vertex();

	Logger::cnt_selected_vertices += mds_context.cnt_sel;
	Logger::cnt_excluded_vertices += mds_context.cnt_excl;
	Logger::cnt_ignored_vertices += mds_context.cnt_ign;
	Logger::cnt_removed_vertices += mds_context.cnt_rem;
	Logger::cnt_dominated_vertices += mds_context.cnt_dom;

	std::unordered_map<int, int> newToOldIndex;
	auto reduced_graph = create_reduced_graph(mds_context,newToOldIndex);

	std::vector<int> solution;
	for (int v = 0; v < mds_context.selected.size(); ++v) {
		if (mds_context.is_selected(v)) {
			//we need a +1 te correct the previous -1.
			solution.push_back(v + 1);
		}
		if (mds_context.is_undetermined(v)){
			Logger::cnt_undetermined_vertices++;
		}
	}

	if (sol_strategy == SOLVER_SAT) {
		timer t_sat;
		std::vector<int> partial_solution = sat_solver_dominating_set(mds_context, reduced_graph, newToOldIndex);

		for (int newIndex : partial_solution) {
			solution.push_back((newToOldIndex[newIndex]) + 1);
		}
		Logger::execution_time_sat += t_sat.count();
	} else {
		throw std::runtime_error("does not yet support other solvers");
	}

	Logger::execution_time_complete = t_complete.count();
	std::cout << Logger::execution_time_complete << std::endl;
	parse::output_solution(solution, path);
	std::string name = parse::getNameFile(path);
	output_loginfo(name);
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
			sub_sub_newToOldIndex[component_u].insert({index_u,newToOldIndex[u]});
			sub_sub_newToOldIndex[component_v].insert({index_v,newToOldIndex[v]});
		} else {
			throw std::runtime_error("Edge endpoints must always be in the same component.");
		}
	}


}

void create_component_subgraphs(const std::string& path,
								std::vector<std::unique_ptr<adjacencyListBoost>>& sub_components,
								std::vector<std::unordered_map<int, int>>& sub_newToOldIndex){

	std::unique_ptr<adjacencyListBoost> graph = std::make_unique<adjacencyListBoost>(parse::load_pace_2024(path));
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
			sub_newToOldIndex[component_u].insert({index_u,u});
			sub_newToOldIndex[component_v].insert({index_v,v});
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
			boost::add_edge(OldToNewIndex[source], OldToNewIndex[target], reduced_graph);
		}
	}
	return reduced_graph;
}


void component_reduction(std::string path)
{
	timer t_complete;
	//create the original graph & context.
	adjacencyListBoost adjLBoost = parse::load_pace_2024(path);

	//run reduction rules & fill the context.
	MDS_CONTEXT mds_context = MDS_CONTEXT(adjLBoost);
	//reduce::reduce_ijcai(mds_context, true);
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
	

