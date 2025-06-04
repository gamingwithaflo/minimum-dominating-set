#pragma once
#include "reduce.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>
#include <iostream>
#include <queue>
#include <absl/strings/str_format.h>
#include <atomic>
#include "util/logger.h"

#include "absl/strings/internal/str_format/extension.h"
#include "util/timer.h"



namespace reduce {
	void reduction_rule_manager(MDS_CONTEXT& mds_context, strategy_reduction& strategy, int l, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration) {
		if (strategy == REDUCTION_COMBINATION) {
			//IJCAI with Alber rule 1.
			reduce_ijcai(mds_context, true, theory_strategy, start, timeout_duration);
		}
		else if (strategy == REDUCTION_ALBER) {
			reduce_alber(mds_context, true);
		}
		else if (strategy == REDUCTION_IJCAI){
			reduce_ijcai(mds_context, false, theory_strategy, start, timeout_duration);
		}
		else if (strategy == REDUCTION_ALBER_RULE_1){
			reduce_alber(mds_context, false);

		} else if (strategy == REDUCTION_NON){
			//Do nothing.
		} else if (strategy == REDUCTION_L_ALBER) {
			reduce_l_alber(mds_context, l, theory_strategy, start, timeout_duration);
		}
	}

	void reduce_alber(MDS_CONTEXT& mds_context, bool run_rule_2) {
		auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
		bool first_time = run_rule_2;
		int cnt_reductions;
		do {
			//reset counter
			cnt_reductions = 0;

			//the first time bool only gets false if, reduce_neigh_pair gets called.
			if (!first_time) {
				auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
			}

			for (auto vertex = vert_itt; vertex < vert_itt_end; ++vertex) {
				//simple reduction rules.
				if (mds_context.is_removed(*vertex)) {
					continue;
				}
				if (mds_context.is_dominated(*vertex)) {
					if (simple_rule_one(mds_context, *vertex)) {
						Logger::cnt_alber_simple_rule_1++;
						++cnt_reductions;
					}
					if (simple_rule_two(mds_context, *vertex)) {
						Logger::cnt_alber_simple_rule_2++;
						++cnt_reductions;
					}
					if (simple_rule_three(mds_context, *vertex)) {
						++cnt_reductions;
					}
					if (simple_rule_four(mds_context, *vertex)) {
						Logger::cnt_alber_simple_rule_4++;
						++cnt_reductions;
					}
				}
			}
			for (auto vertex = vert_itt; vertex < vert_itt_end; ++vertex) {
				if (!mds_context.is_excluded(*vertex) && !mds_context.is_removed(*vertex)) { // if removed is checked above.
					if (reduce_neighborhood_single_vertex(mds_context, *vertex)) {
						++cnt_reductions;
					}
				}
			}
			if (cnt_reductions == 0 && first_time) {
				for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
					if (!mds_context.is_undetermined(*itt)) {
						continue;
					}

					auto possible_combinations = bfs_get_distance_three(mds_context, *itt);
					for (vertex poss : possible_combinations) {
						if (mds_context.is_undetermined(poss)) {
							if (*itt < poss)
							{
								if (reduce_neighborhood_pair_vertices(mds_context, *itt, poss)) {
									++cnt_reductions;
								}
							}
						}
					}
				}
			}
		} while (cnt_reductions > 0);
	}

	void reduce_ijcai(MDS_CONTEXT& mds_context, bool run_rule_2, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration) {
		bool reduced;
		auto [vertex_itt, vertex_itt_end] = mds_context.get_vertices_itt();
		bool first_time = run_rule_2;

		do {
			reduced = false;
			auto [vertex_itt, vertex_itt_end] = mds_context.get_vertices_itt();
			for (auto itt = vertex_itt; itt < vertex_itt_end; ++itt) {
				if (std::chrono::steady_clock::now() - start > timeout_duration){
					return;
				}
				if (mds_context.is_undetermined(*itt)) {
					Logger::attempt_ijcai_rule_1++;
					reduced |= reduce_subset(mds_context,*itt);
				}
				if (!mds_context.is_dominated_ijcai(*itt)) {
					Logger::attempt_ijcai_rule_2++;
					bool temp = reduce_single_dominator(mds_context, *itt);
					reduced |= temp;
					if (temp){
						Logger::cnt_ijcai_rule_2++;
					}
				}
				if (!mds_context.is_dominated_ijcai(*itt)) {
					Logger::attempt_ijcai_rule_3++;
					bool temp = reduce_ignore(mds_context, *itt);
					reduced |= temp;
					if (temp){
						Logger::cnt_ijcai_rule_3++;
					}
				}
			}
			if ((!reduced && first_time)) {
				bool reduction_found = true;
				while (reduction_found)
				{
					reduction_found = false;
					//to prevent a pointer error.
					std::vector<vertex>vertices = mds_context.get_vertices();
					for (auto itt = vertices.begin(); itt < vertices.end(); ++itt) {
						if (std::chrono::steady_clock::now() - start > timeout_duration){
							return;
						}
						if (!mds_context.is_undetermined(*itt)) {
							continue;
						}
						auto possible_combinations = bfs_get_distance_three(mds_context, *itt);
						for (vertex poss : possible_combinations) {
							if (mds_context.is_undetermined(poss)) {
								if (*itt < poss) {
									reduced |= reduce_neighborhood_pair_vertices_ijcai(mds_context, *itt, poss);
								}
							}
						}
					}
				}
				if (!theory_strategy) {
					first_time = false;
				}
			}
		} while (reduced);
	}

	void reduce_l_alber(MDS_CONTEXT& mds_context, int l, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration){
		bool reduction = true;
		auto [first_vert_itt, first_vert_itt_end] = mds_context.get_vertices_itt();
		bool simple_reduce = true;
		while (simple_reduce)
		{
			simple_reduce = false;
			if (std::chrono::steady_clock::now() - start > timeout_duration){
				return;
			}
			for (auto first_vertex = first_vert_itt; first_vertex < first_vert_itt_end; ++first_vertex)
			{
				//simple reduction rules.
				if (mds_context.is_removed(*first_vertex) || (mds_context.is_dominated(*first_vertex) && mds_context.is_excluded(*first_vertex))) {
					continue;
				}
				if (mds_context.is_dominated(*first_vertex)) {
					if (simple_rule_one(mds_context, *first_vertex)) {
						simple_reduce = true;
					}
					if (simple_rule_two(mds_context, *first_vertex)) {
						simple_reduce = true;
					}
					if (simple_rule_three(mds_context, *first_vertex)) {
						simple_reduce = true;
					}
					if (simple_rule_four(mds_context, *first_vertex)) {
						simple_reduce = true;
					}
				}
			}
		}
		while (reduction)
		{
			//boost::unordered_set<std::vector<int>> done;
			reduction = false;
			auto [vert_it, vert_it_end] = mds_context.get_vertices_itt();
			for (;vert_it != vert_it_end; ++vert_it) {
				if (std::chrono::steady_clock::now() - start > timeout_duration){
					return;
				}
				if (mds_context.is_undetermined(*vert_it)) {
					//std::cout << "iteration: " << counter << std::endl;
					reduction |= execute_l_alber(mds_context, l, 1, { static_cast<int>(*vert_it) });
					//execute_l_alber_two(mds_context, l, *vert_it);
				}
			}
			if (!theory_strategy){
				//std::cout << "stop theory" << std::endl;
				reduction = false;
			}
		}
		// this makes the next reduction round faster (or finds small improvements).
		 int smaller_l = 1;
		 //std::cout << "reduction" << reduction << std::endl;
		 while (smaller_l <= l && theory_strategy) {
		 	//std::cout << "test" << std::endl;
		 	//boost::unordered_set<std::vector<int>> done_2;
		 	bool reduction_smaller = true;
		 	bool found_a_reduction = false;
		 	while (reduction_smaller){
		 		reduction_smaller = false;
		 	auto [vert_it, vert_it_end] = mds_context.get_vertices_itt();
		 	for (auto itt = vert_it; itt < vert_it_end; ++itt)
		 	{
		 		if (std::chrono::steady_clock::now() - start > timeout_duration){
		 			return;
		 		}
		 		if (mds_context.is_undetermined(*vert_it)) {
		 			bool succes = execute_l_alber(mds_context, smaller_l, 1, { static_cast<int>(*vert_it) });
		 			reduction_smaller |= succes;
		 			if (succes) {
		 				found_a_reduction = true;
		 				bool simple_reduce_2 = true;
		 				while (simple_reduce_2)
		 				{
		 					simple_reduce_2 = false;
		 					auto [vert_itt_four, vert_itt_four_end] = mds_context.get_vertices_itt();
		 					for (auto vertex_four = vert_itt_four_end; vertex_four < vert_itt_four_end; ++vertex_four)
		 					{
		 						//simple reduction rules.
		 						if (mds_context.is_removed(*vertex_four) || (mds_context.is_dominated(*vertex_four) && mds_context.is_excluded(*vertex_four))) {
		 							continue;
		 						}
		 						if (mds_context.is_dominated(*vertex_four)) {
		 							if (simple_rule_one(mds_context, *vertex_four)) {
		 								simple_reduce_2 = true;
		 							}
		 							if (simple_rule_two(mds_context, *vertex_four)) {
		 								simple_reduce_2 = true;
		 							}
		 							if (simple_rule_three(mds_context, *vertex_four)) {
		 								simple_reduce_2 = true;
		 							}
		 							if (simple_rule_four(mds_context, *vertex_four)) {
		 								simple_reduce_2 = true;
		 							}
		 						}
		 					}
		 				}
		 			}
		 		}
		 	}
		 	}
		 	if (found_a_reduction) {
		 		std::cout << "found something" << std::endl;
		 		smaller_l = 1;
		 	} else {
		 		smaller_l = smaller_l + 1;
		 	}
		 }
	}

	void combineRecursive(const std::vector<int>& input, int k, int start,
					  std::vector<int>& current, MDS_CONTEXT& mds_context) {
		if (current.size() == k) {
			std::cout << "[";
			for (size_t i = 0; i < current.size(); ++i) {
				std::cout << current[i];
				if (i != current.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "]" << std::endl;
			bool reduction = reduction_l_rule(mds_context, current);
			if (reduction) {
				std::cout << "found one" << std::endl;
				// auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
				// for (auto vertex = vert_itt; vertex < vert_itt_end; ++vertex)
				// {
				// 	//simple reduction rules.
				// 	if (mds_context.is_removed(*vertex) || (mds_context.is_dominated(*vertex) && mds_context.is_excluded(*vertex))) {
				// 		continue;
				// 	}
				// 	if (mds_context.is_dominated(*vertex)) {
				// 		if (simple_rule_one(mds_context, *vertex)) {
				// 			Logger::cnt_alber_simple_rule_1++;
				// 		}
				// 		if (simple_rule_two(mds_context, *vertex)) {
				// 			Logger::cnt_alber_simple_rule_2++;
				// 		}
				// 		if (simple_rule_three(mds_context, *vertex)) {
				// 		}
				// 		if (simple_rule_four(mds_context, *vertex)) {
				// 			Logger::cnt_alber_simple_rule_4++;
				// 		}
				// 	}
				// }
			}
			return;
		}

		for (int i = start; i <= input.size() - (k - current.size()); ++i) {
			current.push_back(input[i]);
			combineRecursive(input, k, i + 1, current, mds_context);
			//std::cout << current[0] << std::endl;
			std::cout << "[";
			for (size_t i = 0; i < current.size(); ++i) {
				std::cout << current[i];
				if (i != current.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "]" << std::endl;
			std::cout << "pop_back" << std::endl;
			current.pop_back(); // backtrack
		}
		std::cout << "end" << current.size() << std::endl;
	}

	std::vector<std::vector<int>> generateCombinations(const std::vector<int>& input, int k, MDS_CONTEXT& mds_context) {
		std::vector<int> current;
		combineRecursive(input, k, 0, current, mds_context);
	}

	void reduce_l_alber_dense(MDS_CONTEXT& mds_context, int l, std::atomic<bool>& stop_flag){

		auto [vert_it, vert_it_end] = mds_context.get_vertices_itt();
		int counter = 0;
		std::vector<int> undetermined_vertices;
		for (;vert_it != vert_it_end; ++vert_it)
		{
			if (stop_flag){
				return;
			}
			if (mds_context.is_undetermined(*vert_it)) {
				std::cout << "undetermined vertex: " << *vert_it << std::endl;
				undetermined_vertices.push_back(*vert_it);
			}
		}
		std::cout << undetermined_vertices.size() << std::endl;
		generateCombinations(undetermined_vertices, l, mds_context);
		std::cout << "is it after." << std::endl;
		//this makes the next reduction round faster (or finds small improvements).
	}

	bool hasDuplicate(const std::vector<int>& nums) {
		std::unordered_set<int> seen;
		for (int num : nums) {
			if (seen.count(num)) return true; // duplicate found
			seen.insert(num);
		}
		return false; // no duplicates
	}

	bool execute_l_alber(MDS_CONTEXT& mds_context, int l, int counter, std::vector<int> vertices){
		if (l == counter){
			//std::sort(vertices.begin(), vertices.end());
			//if (done.find(vertices) == done.end())
			//{
				//done.insert(vertices);
				Logger::attempt_alber_l_reduction++;
				bool reduction = reduction_l_rule(mds_context, vertices);
				if (reduction){
					bool simple_reduction = true;
					while (simple_reduction)
					{
						simple_reduction = false;
						auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
						for (auto vertex = vert_itt; vertex < vert_itt_end; ++vertex)
						{
							//simple reduction rules.
							if (mds_context.is_removed(*vertex) || (mds_context.is_dominated(*vertex) && mds_context.is_excluded(*vertex))) {
								continue;
							}
							if (mds_context.is_dominated(*vertex)) {
								if (simple_rule_one(mds_context, *vertex)) {
									Logger::cnt_alber_simple_rule_1++;
									simple_reduction = true;
								}
								if (simple_rule_two(mds_context, *vertex)) {
									Logger::cnt_alber_simple_rule_2++;
									simple_reduction = true;
								}
								if (simple_rule_three(mds_context, *vertex)) {
									simple_reduction = true;
								}
								if (simple_rule_four(mds_context, *vertex)) {
									Logger::cnt_alber_simple_rule_4++;
									simple_reduction = true;
								}
							}
						}
					}
					return true;
				}
				return false;
			//}
			return false;
		}
		bool reduced = false;
		auto possible_combination = bfs_get_distance_three_generalized(mds_context, vertices);
		for (auto& vtx : possible_combination){
			if (mds_context.is_undetermined(vtx)) {
				std::vector<int> updated_vertices = vertices;
				updated_vertices.push_back(vtx);
				//dont do double work.
				bool is_biggest = true;
				for (auto& vert : vertices) {
					if (vert < vtx){
						continue;
					}
					is_biggest = false;
				}
				if (is_biggest){
					reduced |= execute_l_alber(mds_context, l, counter + 1, updated_vertices);
				}
				// if (*std::min_element(updated_vertices.begin(), updated_vertices.end()) == vtx){
				// // 	std::cout << vtx << std::endl;
				// reduced |= execute_l_alber(mds_context, l, counter+1, updated_vertices);
				// }
				//reduced |= execute_l_alber(mds_context, l, counter+1, updated_vertices, done);
			}
		}
		return reduced;
	}

	void execute_l_alber_two(MDS_CONTEXT& mds_context, int l, int vertex) {
		auto possible_combination = bfs_get_distance_three(mds_context, vertex);
		int n = possible_combination.size();
		for (int mask = 1; mask < (1 << n); ++mask) {
			std::vector<int> subset;
			for (int i = 0; i < n; ++i) {
				if (mask & (1 << i)) {
					subset.push_back(possible_combination[i]);
				}
			}
			if (subset.size() == l-1) {
				subset.push_back(vertex);
				Logger::attempt_alber_l_reduction++;
				bool reduction = reduction_l_rule(mds_context, subset);
			}
		}
	}


	std::vector<vertex> bfs_get_distance_three_generalized(MDS_CONTEXT& mds_context, std::vector<int>& vertices) {
		std::unordered_set<vertex> visited;
		std::vector<vertex> within_distance_three;
		std::queue<std::pair<vertex,int>> queue;

		for (auto& v : vertices) {
			visited.insert(v);
			queue.emplace(v,0);
		}
		//queue.emplace(vertices[0],0);

		while (!queue.empty()){
			auto [current, depth] = queue.front();
			queue.pop();

			if (depth >= 3) continue;

			auto [neigh_it, neigh_end] = mds_context.get_neighborhood_itt(current);
			for (; neigh_it < neigh_end; ++neigh_it) {
				vertex neighbor = *neigh_it;

				if (!visited.count(neighbor)){
					visited.insert(neighbor);
					if (mds_context.is_undetermined(neighbor))
					{
						within_distance_three.push_back(neighbor);
					}
					queue.emplace(neighbor,depth+1);
				}
			}
		}
		std::reverse(within_distance_three.begin(), within_distance_three.end());
		return within_distance_three;

	}

	std::vector<vertex> bfs_get_distance_three(MDS_CONTEXT& mds_context, vertex v){
		std::unordered_set<vertex> visited;
		std::vector<vertex> within_distance_three;
		std::queue<std::pair<vertex,int>> queue;

		visited.insert(v);
		queue.emplace(v,0);

		while (!queue.empty()){
			auto [current, depth] = queue.front();
			queue.pop();

			if (depth == 3) continue;

			auto [neigh_it, neigh_end] = mds_context.get_neighborhood_itt(current);
			for (; neigh_it < neigh_end; ++neigh_it) {
				vertex neighbor = *neigh_it;
				if (mds_context.is_excluded(neighbor) && mds_context.is_dominated(neighbor)) {
					continue;
				}
				if (!visited.count(neighbor)){
					visited.insert(neighbor);
					if (mds_context.is_undetermined(neighbor)){
						within_distance_three.push_back(neighbor);
					}
					queue.emplace(neighbor,depth+1);
				}
			}
		}
		return within_distance_three;

	}

	bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u) {
		bool is_reduced = false;
		
		//Get neighborhood.
		auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(u);

		//create neighborhood lookup table
		int num_vertices = mds_context.get_total_vertices();
		std::vector<int>lookup = std::vector<int>(num_vertices, 0);
		lookup[u] = 1;
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			lookup[*v] = 1;
		}

		std::unordered_set<int>exit_vertices; //N_{3}
		std::vector<int>guard_vertices; //N_{2}
		std::vector<int>prison_vertices; //N_{1}

		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//for each vertex v get the neighborhood.
			auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(*v);
			for (; neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				//If there is at least 1 neighbor is not within the neighborhood of u. It becomes a exit vertex.
				if (lookup[*neigh_itt_v] == 0) {
					exit_vertices.insert(*v);
					break;
				}
			}
		}
		//Identify if remaining vertices go into guard_vertices or prison_vertices.
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//check if vertex is not a exit_vertex.
			if (exit_vertices.count(*v)) {
				continue;
			}
			bool guard_trigger = false;
			auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(*v);
			for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				//Check if it is a guard_vertex.
				if (exit_vertices.count(*neigh_itt_v)) {
					guard_vertices.push_back(*v);
					guard_trigger = true;
					break;
				}
			}
			//Not adjacent to exit_vertex then it is a: prison vertex.
			if (guard_trigger == false) {
				prison_vertices.push_back(*v);
			}
		}

		//Check whether the graph can be reduced.
		if (mds_context.can_be_reduced(prison_vertices)) {
			is_reduced = true;
			++Logger::cnt_alber_rule_1_default;
			mds_context.select_vertex(u);
			//Remove all Prison vertices for complete graph.
			for (auto itt : prison_vertices) {
				mds_context.remove_vertex(itt);
			}
			for (auto itt : guard_vertices) {
				mds_context.remove_vertex(itt);
			}
			for (auto itt : exit_vertices) {
				mds_context.dominate_vertex(itt); //should already be done.
			}
			mds_context.remove_vertex(u);
		}
		else if (!guard_vertices.empty()) {
			for (auto itt : guard_vertices) {
				if (!mds_context.is_excluded(itt)) {
					mds_context.exclude_vertex(itt);
					is_reduced = true;
				}
			}
			if (is_reduced){
				Logger::cnt_alber_rule_1_guard++;
			}
		}
		return is_reduced;
	}
	
	bool reduce_neighborhood_pair_vertices(MDS_CONTEXT& mds_context, vertex v, vertex w) {
		{
			if (mds_context.is_removed(v) || mds_context.is_removed(w) || mds_context.is_excluded(v) || mds_context.is_excluded(w)) {
				return false;
			}
			// get adjacency lookup table & itteratable list of all neighbors of v & w (lookup includes v&w while itt. excludes them).
			auto [lookup, pair_neighborhood_vector] = mds_context.get_pair_neighborhood(v, w);

			// partition neighborhood u into 3 sets.
			std::vector<int>exit_vertices; //N_{3}
			std::vector<int>guard_vertices; //N_{2}
			std::vector<int>prison_vertices; //N_{1}

			//Identify exit_vertices
			for (auto u = pair_neighborhood_vector.begin(); u < pair_neighborhood_vector.end(); ++u) {
				//for each vertex get the neighborhood
				auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
				//if ANY neighbor isn't in lookup (it belongs to exit_vertices).
				for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
					if (lookup[*neigh_itt_u] == 0) {
						exit_vertices.push_back(*u);
						break;
					}
				}
			}
			//divide all non N_exit vertices into N_guard and N_prison. (could be abstracted).
			for (auto u = pair_neighborhood_vector.begin(); u < pair_neighborhood_vector.end(); ++u) {
				//check if vertex is not a exit_vertex.
				if (std::find(exit_vertices.begin(), exit_vertices.end(), *u) != exit_vertices.end()) {
					continue;
				}
				else {
					bool guard_trigger = false;
					//check if a neighbor vertex is adjacent to a exit_vertex.
					auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
					for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
						//Check if it is a guard_vertex.
						if (std::find(exit_vertices.begin(), exit_vertices.end(), *neigh_itt_u) != exit_vertices.end()) {
							guard_vertices.push_back(*u);
							guard_trigger = true;
							break;
						}
					}
					//Not adjacent to exit_vertex then it is a: prison vertex.
					if (guard_trigger == false) {
						prison_vertices.push_back(*u);
					}
				}
			}

			//find the subset of undominated N_prison vertices.
			std::vector<int>undominated_prison_vertices;
			for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
				if (mds_context.dominated[*i] == 0) {
					undominated_prison_vertices.push_back(*i);
				}
			}
			//Is there guaranteed profit.
			if (undominated_prison_vertices.size() > 0) {
				int size = undominated_prison_vertices.size();
				//create domination table. (how many vertices of undominated_prison_vertices can a vertex dominate).
				int num_vertices = mds_context.get_total_vertices();
				std::vector<int>domination = std::vector<int>(num_vertices, 0);
				for (auto i = undominated_prison_vertices.begin(); i < undominated_prison_vertices.end(); ++i) {
					domination[*i]++; //dominate himself.
					auto [edge_itt, edge_itt_end] = mds_context.get_neighborhood_itt(*i);
					for (;edge_itt, edge_itt < edge_itt_end; ++edge_itt) {
						domination[*edge_itt]++;
					}
				}
				//Check if undominated N_prison can be dominated by a single N_prison.
				for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
					if (domination[*i] == size) {
						return false;
					}
				}
				//Check if undominated N_prison can be dominated by a single N_guard.
				for (auto i = guard_vertices.begin(); i < guard_vertices.end(); ++i) {
					if (domination[*i] == size) {
						return false;
					}
				}
				//Check if only v, dominates all undominated N_prison vertices.
				bool dominated_by_v = (domination[v] == size);
				//Check if only w, dominates all undominated N_prison vertices.
				bool dominated_by_w = (domination[w] == size);
				//divide the cases.
				if (dominated_by_v && dominated_by_w) {
					Logger::cnt_alber_rule_2_either++;
					//the optimal is either to choose v, w (or both)

					//Create gadget which forces either v, w or both. (because we excluded this vertex, you dont need the gadget)
					vertex z1 = mds_context.add_vertex();

					mds_context.add_edge(v, z1);
					mds_context.add_edge(w, z1);

					//remove prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.remove_vertex(*i);
					}
					for (auto i = guard_vertices.begin(); i < guard_vertices.end(); ++i) {
						auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
						auto [neigh_itt_w, neigh_itt_w_end] = mds_context.get_neighborhood_itt(w);
						if (std::find(neigh_itt_v, neigh_itt_v_end, *i) != neigh_itt_v_end && std::find(neigh_itt_w, neigh_itt_w_end, *i) != neigh_itt_w_end) {
							mds_context.remove_vertex(*i);
						}
					}

					//should not actually be excluded: just do it.
					mds_context.exclude_vertex(z1);

					return true;
				}
				if (dominated_by_v) {
					// the optimal is to choose v.
					Logger::cnt_alber_rule_2_single++;
					mds_context.select_vertex(v);
					//remove all prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.remove_vertex(*i);
					}

					auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
					for (; neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
						//dominate neighborhood of w, because it gets included.
						if (std::find(guard_vertices.begin(), guard_vertices.end(), *neigh_itt_v) != guard_vertices.end()) {
							//if it is a guard it can be removed.
							mds_context.remove_vertex(*neigh_itt_v);
						}
					}
					mds_context.remove_vertex(v);
					return true;
				}
				if (dominated_by_w) {
					// the optimal is to choose w.
					Logger::cnt_alber_rule_2_single++;
					mds_context.select_vertex(w);
					//remove all prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.remove_vertex(*i);
					}

					auto [neigh_itt_w, neigh_itt_w_end] = mds_context.get_neighborhood_itt(w);
					for (; neigh_itt_w < neigh_itt_w_end; ++neigh_itt_w) {
						//dominate neighborhood of w, because it gets included.
						mds_context.dominate_vertex(*neigh_itt_w);
						if (std::find(guard_vertices.begin(), guard_vertices.end(), *neigh_itt_w) != guard_vertices.end()) {
							//if it is a guard it can be removed.
							mds_context.remove_vertex(*neigh_itt_w);
						}
					}
					mds_context.remove_vertex(w);
					return true;
				}
				//the optimal is to choose both v & w.
				Logger::cnt_alber_rule_2_both++;
				mds_context.select_vertex(v);
				mds_context.select_vertex(w);

				//dominate pair_neighborhood
				for (auto u = prison_vertices.begin(); u < prison_vertices.end(); ++u) {
					mds_context.remove_vertex(*u);
				}
				for (auto u = guard_vertices.begin(); u < guard_vertices.end(); ++u) {
					mds_context.remove_vertex(*u);
				}

				mds_context.remove_vertex(v);
				mds_context.remove_vertex(w);
				return true;
			}
			//We cannot give guarantees.
			return false;
		}
	}

	bool reduce_neighborhood_pair_vertices_ijcai(MDS_CONTEXT& mds_context, vertex v, vertex w) {
		{
			//either already selected, or 
			if (mds_context.is_excluded(v) || mds_context.is_excluded(w) || mds_context.is_selected(v) || mds_context.is_selected(w)) {
				return false;
			}
			// get adjacency lookup table & itteratable list of all neighbors of v & w (lookup includes v&w while itt. excludes them).
			auto [lookup, pair_neighborhood_vector] = mds_context.get_pair_neighborhood(v, w);

			// partition neighborhood u into 3 sets.
			std::vector<int>exit_vertices; //N_{3}
			std::vector<int>guard_vertices; //N_{2}
			std::vector<int>prison_vertices; //N_{1}

			for (auto u = pair_neighborhood_vector.begin(); u < pair_neighborhood_vector.end(); ++u) {
				//for each vertex get the neighborhood
				auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
				//if ANY neighbor isn't in lookup (it belongs to exit_vertices).
				for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
					if (lookup[*neigh_itt_u] == 0) {
						if (mds_context.is_dominated(*u) && mds_context.is_dominated(*neigh_itt_u)) {
							continue;
						}
						if (mds_context.is_dominated(*neigh_itt_u) && mds_context.is_excluded(*neigh_itt_u)) {
							continue;
						}
						if (mds_context.is_selected(*neigh_itt_u)) {
							continue;
						}
						exit_vertices.push_back(*u);
						break;
					}
				}
			}
			//divide all non N_exit vertices into N_guard and N_prison. (could be abstracted).
			for (auto u = pair_neighborhood_vector.begin(); u < pair_neighborhood_vector.end(); ++u) {
				//check if vertex is not a exit_vertex.
				if (std::find(exit_vertices.begin(), exit_vertices.end(), *u) != exit_vertices.end()) {
					continue;
				}
				else {
					bool guard_trigger = false;
					//check if a neighbor vertex is adjacent to a exit_vertex.
					auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
					for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
						//Check if it is a guard_vertex.
						if (std::find(exit_vertices.begin(), exit_vertices.end(), *neigh_itt_u) != exit_vertices.end()) {
							guard_vertices.push_back(*u);
							guard_trigger = true;
							break;
						}
					}
					//Not adjacent to exit_vertex then it is a: prison vertex.
					if (guard_trigger == false) {
						prison_vertices.push_back(*u);
					}
				}
			}

			//find the subset of undominated N_prison vertices.
			std::vector<int>undominated_prison_vertices;
			for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
				if (!mds_context.is_dominated(*i)) {
					undominated_prison_vertices.push_back(*i);
				}
			}

			//Is there guaranteed profit.
			if (undominated_prison_vertices.size() > 0) {
				int size = undominated_prison_vertices.size();
				//create domination table. (how many vertices of undominated_prison_vertices can a vertex dominate).
				int num_vertices = mds_context.get_total_vertices();
				std::vector<int>domination = std::vector<int>(num_vertices, 0);
				for (auto i = undominated_prison_vertices.begin(); i < undominated_prison_vertices.end(); ++i) {
					domination[*i]++; //dominate himself.
					auto [edge_itt, edge_itt_end] = mds_context.get_neighborhood_itt(*i);
					for (;edge_itt, edge_itt < edge_itt_end; ++edge_itt) {
						domination[*edge_itt]++;
					}
				}
				//Check if undominated N_prison can be dominated by a single N_prison.
				for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
					if (domination[*i] == size) {
						return false;
					}
				}
				//Check if undominated N_prison can be dominated by a single N_guard.
				for (auto i = guard_vertices.begin(); i < guard_vertices.end(); ++i) {
					if (domination[*i] == size) {
						return false;
					}
				}
				//Check if only v, dominates all undominated N_prison vertices.
				bool dominated_by_v = (domination[v] == size);
				//Check if only w, dominates all undominated N_prison vertices.
				bool dominated_by_w = (domination[w] == size);
				//divide the cases.
				if (dominated_by_v && dominated_by_w) {
					++Logger::cnt_alber_rule_2_either;
					//the optimal is either to choose v, w (or both)

					//Create gadget which forces either v, w or both. when created, automatically excluded.
					vertex z1 = mds_context.add_vertex();

					mds_context.add_edge(v, z1);
					mds_context.add_edge(w, z1);

					//remove prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.exclude_vertex(*i);
						mds_context.dominate_vertex(*i);
					}
					//you can remove vertices which are both in the neighborhood of v & w.
					for (auto i = guard_vertices.begin(); i < guard_vertices.end(); ++i) {
						auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
						auto [neigh_itt_w, neigh_itt_w_end] = mds_context.get_neighborhood_itt(w);
						if (std::find(neigh_itt_v, neigh_itt_v_end, *i) != neigh_itt_v_end && std::find(neigh_itt_w, neigh_itt_w_end, *i) != neigh_itt_w_end) {
							mds_context.exclude_vertex(*i);
							mds_context.dominate_vertex(*i);
						}
					}

					mds_context.exclude_vertex(z1);
					return true;
				}
				if (dominated_by_v) {
					// the optimal is to choose v.
					mds_context.select_vertex(v);
					++Logger::cnt_alber_rule_2_single;
					//remove all prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.exclude_vertex(*i);
					}

					auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
					for (; neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
						//neighborhood of v gets dominated by select_vertex(v).
						if (std::find(guard_vertices.begin(), guard_vertices.end(), *neigh_itt_v) != guard_vertices.end()) {
							//if it is a guard it can be removed.
							mds_context.exclude_vertex(*neigh_itt_v);
						}
					}
					return true;
				}
				if (dominated_by_w) {
					// the optimal is to choose w.
					++Logger::cnt_alber_rule_2_single;
					mds_context.select_vertex(w);
					//remove all prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.exclude_vertex(*i);
					}

					auto [neigh_itt_w, neigh_itt_w_end] = mds_context.get_neighborhood_itt(w);
					for (; neigh_itt_w < neigh_itt_w_end; ++neigh_itt_w) {
						//neighborhood of w gets dominated by select_vertex(w).
						if (std::find(guard_vertices.begin(), guard_vertices.end(), *neigh_itt_w) != guard_vertices.end()) {
							//exclude guard vertices.
							mds_context.exclude_vertex(*neigh_itt_w);
						}
					}
					return true;
				}
				//the optimal is to choose both v & w.
				++Logger::cnt_alber_rule_2_both;
				mds_context.select_vertex(v);

				mds_context.select_vertex(w);

				//dominate pair_neighborhood
				for (auto u = prison_vertices.begin(); u < prison_vertices.end(); ++u) {
					mds_context.exclude_vertex(*u);
				}
				for (auto u = guard_vertices.begin(); u < guard_vertices.end(); ++u) {
					mds_context.exclude_vertex(*u);
				}
				return true;
			}
			//We cannot give guarantees.
			return false;
		}
	}

	//remove edges between dominated vertices. (NOT IGNORED, only actual dominated vertices)
	bool simple_rule_one(MDS_CONTEXT& mds_context, vertex v) {
		//given the vertex is dominated.
		auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v);
		bool reduced = false;
		for (;neigh_v_itt < neigh_v_itt_end; ++neigh_v_itt) {
			if (mds_context.is_dominated(*neigh_v_itt)) {
				mds_context.remove_edge(v, *neigh_v_itt);
				reduced = true;
			}
		}
		return reduced;
	}

	//works only for dominated vertices.
	bool simple_rule_two(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.is_removed(v)) {
			return false;
		}

		if (mds_context.get_out_degree_vertex(v) == 1)
		{
			auto [it_neigh, it_neigh_end] = mds_context.get_neighborhood_itt(v);
			if (mds_context.is_undetermined(*it_neigh)){
				mds_context.removed[v] = true;
				mds_context.excluded[v] = true;
				return true;
			}
			return false;
		}
		return false;
	}

	bool simple_rule_three(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.is_removed(v)) {
			return false;
		}
		//in principle could u1 & u2 not be dominated because all edges between dominated vertices should be removed by a previous rule.
		if (mds_context.get_out_degree_vertex(v) == 2) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			//Not both vertices should be excluded.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			if (!mds_context.is_undetermined(u_one) || !mds_context.is_undetermined(u_two)) {
				return false;
			}
			//the rule is for back black neighboring vertices. (should already not be possible after simple rule 1 but if that one is not on.
			if (mds_context.is_dominated(u_one) || mds_context.is_dominated(u_two) ) {
				return false;
			}
			//rule 3.1
			if (mds_context.edge_exists(u_one, u_two)) {
				mds_context.removed[v] = true;
				mds_context.excluded[v] = true;
				boost::clear_vertex(v, mds_context.graph);
				++Logger::cnt_alber_simple_rule_3dot1;
				return true;
			}
			//rule 3.2
			auto [neigh_u_one_itt, neigh_u_one_itt_end] = mds_context.get_neighborhood_itt(u_one);
			for (;neigh_u_one_itt < neigh_u_one_itt_end; ++neigh_u_one_itt) {
				//u_2 must be not dominated & not excluded (or undetermined) (is allowed to be ignored /dominated)
				if (*neigh_u_one_itt != v && mds_context.edge_exists(*neigh_u_one_itt, u_two) && mds_context.is_undetermined(*neigh_u_one_itt)) { //pretty sure this is a bug.
					mds_context.removed[v] = true;
					mds_context.excluded[v] = true;
					boost::clear_vertex(v, mds_context.graph);
					++Logger::cnt_alber_simple_rule_3dot2;
					return true;
				}
			}
			//no intersection found.
			return false;
		}
		return false;
	}

	bool simple_rule_four(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.is_removed(v)) {
			return false;
		}

		if (mds_context.get_out_degree_vertex(v) == 3) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			vertex u_three = *++neigh_v_itt;
			if (mds_context.is_dominated(u_one) || mds_context.is_dominated(u_two) || mds_context.is_dominated(u_three)) {
				return false;
			}
			//rule 4
			bool exists = mds_context.edge_exists(u_one, u_two);
			auto exists_2 = mds_context.edge_exists(u_two, u_three);
			if (exists && exists_2 && mds_context.is_undetermined(u_one) && mds_context.is_undetermined(u_two) && mds_context.is_undetermined(u_three)) {
				mds_context.removed[v] = true;
				mds_context.excluded[v] = true;
				boost::clear_vertex(v, mds_context.graph);
				return true;
			}
			return false;
		}
		return false;
	}

	//Helper function: does the closed neighborhood of v, contain u.
	bool contains(MDS_CONTEXT& mds_context, vertex v, vertex u) {
		if (v == u) {
			return true;
		}
		auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
		if (!std::binary_search(neigh_itt_v, neigh_itt_v_end, u)) {
			return false;
		}
		return true;
	}

	bool check_subset(MDS_CONTEXT& mds_context, vertex v) {
		//selecting this vertex doesn't provide new dominated vertices. (so it can be excluded)
		if (mds_context.get_coverage_size(v) == 0) {
			return true;
		}

		auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
		int minimum_frequency = std::numeric_limits<int>::max();
		vertex minimum_frequency_vertex;

		// Find the vertex in N[u] - N[S] with the lowest frequency.
		for (auto itt = neigh_itt_v ; itt < neigh_itt_v_end; ++itt) {
			if (!mds_context.is_dominated_ijcai(*itt) && (mds_context.get_frequency(*itt) < minimum_frequency)) {
				minimum_frequency_vertex = *itt;
				minimum_frequency = mds_context.get_frequency(*itt);
			}
		}
		//to make it a closed neighborhood.
		if (!mds_context.is_dominated_ijcai(v) && (mds_context.get_frequency(v) < minimum_frequency)) {
			minimum_frequency_vertex = v;
			minimum_frequency = mds_context.get_frequency(v);
		}

		//find vertices which needs to be dominated.
		std::vector<vertex> needs_to_dominate;
		if (!mds_context.is_dominated_ijcai(v) && v != minimum_frequency_vertex) {
			needs_to_dominate.push_back(v);
		}
		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt) {
			if (minimum_frequency_vertex != *itt && !mds_context.is_dominated_ijcai(*itt)) {
				needs_to_dominate.push_back(*itt);
			}
		}

		//get all possible vertices which could be the superset.
		auto [neigh_itt_mf, neigh_itt_mf_end] = mds_context.get_neighborhood_itt(minimum_frequency_vertex);
		for (auto itt = neigh_itt_mf; itt < neigh_itt_mf_end; ++itt) {
			if (*itt == v || !mds_context.is_undetermined(*itt)) {
				continue;
			}
			// Check whether all vertices which needs to be dominated are adjacent to itt.
			bool fail = false;
			for (vertex dominated : needs_to_dominate) {
				if (!contains(mds_context, dominated, *itt)) {
					fail = true;
					break;
				}
			}
			if (!fail) {
				return true;
			}
		}
		//minimum_frequency_vertex is also a contender for being the superset.
		if (minimum_frequency_vertex == v || !mds_context.is_undetermined(minimum_frequency_vertex)) {
			return false;
		}
		bool fail = false;
		for (vertex dominated : needs_to_dominate) {
			if (!contains(mds_context, dominated, minimum_frequency_vertex)) {
				fail = true;
				break;
			}
		}
		if (!fail) {
			return true;
		}
		else {
			return false;
		}
	}

	bool reduce_subset(MDS_CONTEXT& mds_context, vertex v) {
		if (check_subset(mds_context, v)) {
			mds_context.exclude_vertex(v);
			Logger::cnt_ijcai_rule_1++;
			return true;
		}
		return false;
	}

	bool reduce_ignore(MDS_CONTEXT& mds_context, vertex v) {
		bool reduced = false;

		//is a check of mds_context.get_frequency(v) >= 1 needed?

		//find the vertex in n[u] for which the coverage is minimized
		auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
		vertex minimum_coverage_vertex;
		int minimum_coverage = std::numeric_limits<int>::max();

		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt) {
			if (!mds_context.is_excluded(*itt) && (mds_context.get_coverage_size(*itt) < minimum_coverage)) {
				minimum_coverage = mds_context.get_coverage_size(*itt);
				minimum_coverage_vertex = *itt;
			}
		}
		if (!mds_context.is_excluded(v) && mds_context.get_coverage_size(v) < minimum_coverage) {
			minimum_coverage = mds_context.get_coverage_size(v);
			minimum_coverage_vertex = v;
		}

		std::vector<vertex>need_to_cover;
		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt) {
			if (minimum_coverage_vertex != *itt && !mds_context.is_excluded(*itt)) {
				need_to_cover.push_back(*itt);
			}
		}
		if (minimum_coverage_vertex != v && !mds_context.is_excluded(v)) {
			need_to_cover.push_back(v);
		}

		auto [neigh_itt_mc, neigh_itt_mc_end] = mds_context.get_neighborhood_itt(minimum_coverage_vertex);
		for (auto itt = neigh_itt_mc; itt < neigh_itt_mc_end; ++itt) {
			if (*itt == v || mds_context.is_dominated_ijcai(*itt) || mds_context.get_frequency(*itt) < mds_context.get_frequency(v)) {
				continue;
			}
			bool fail = false;
			for (vertex cover : need_to_cover) {
				if (!contains(mds_context, cover, *itt)) {
					fail = true;
					break;
				}
			}
			if (!fail) {
				mds_context.ignore_vertex(*itt);
				reduced = true;
			}
		}
		if (minimum_coverage_vertex == v || mds_context.is_dominated_ijcai(minimum_coverage_vertex) || mds_context.get_frequency(minimum_coverage_vertex) < mds_context.get_frequency(v)) {
			return reduced;
		}
		else {
			bool fail = false;
			for (vertex cover : need_to_cover) {
				if (!contains(mds_context, cover, minimum_coverage_vertex)) {
					fail = true;
					break;
				}
			}
			if (!fail) {
				mds_context.ignore_vertex(minimum_coverage_vertex);
				reduced = true;
			}
			return reduced;
		}
	}

	bool reduce_single_dominator(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.get_frequency(v) != 1) {
			return false;
		}
		if (mds_context.is_undetermined(v)) {
			mds_context.select_vertex(v);
			return true;
		}
		auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
		for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
			if (!mds_context.is_undetermined(*neigh_itt_v)) {
				continue;
			}
			else {
				mds_context.select_vertex(*neigh_itt_v);
				break;
			}
		}
		return true;
	}

	bool reduction_l_rule(MDS_CONTEXT& mds_context, std::vector<int>& l_vertices) {
		for (vertex v : l_vertices) {
			if (mds_context.is_removed(v) || mds_context.is_excluded(v))
			{
				return false;
			}
		}
		//get l vertices.

		//get neighbourhood l.
		timer t_divide_neighborhood;
		std::unordered_set<int> lookup_neighbourhood;
		std::vector<int> l_neighbourhood;
		mds_context.get_l_neighborhood(l_vertices, lookup_neighbourhood, l_neighbourhood);

		// partition neighborhood u into 3 sets.
		std::vector<int>exit_vertices; //N_{3}
		std::vector<int>guard_vertices; //N_{2}
		std::vector<int>prison_vertices; //N_{1}

		// get N_exit(V_l)

		for (auto u = l_neighbourhood.begin(); u < l_neighbourhood.end(); ++u) {
			//for each vertex get the neighborhood
			auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
			//if ANY neighbor isn't in lookup (it belongs to exit_vertices).
			for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
				if (lookup_neighbourhood.find(*neigh_itt_u) == lookup_neighbourhood.end()) {
					if (mds_context.is_dominated(*u) && mds_context.is_dominated(*neigh_itt_u)) {
						continue;
					}
					if (mds_context.is_dominated(*neigh_itt_u) && mds_context.is_excluded(*neigh_itt_u)) {
						continue;
					}
					if (mds_context.is_selected(*neigh_itt_u)) {
						continue;
					}
					exit_vertices.push_back(*u);
					break;
				}
			}
		}
		//divide all non N_exit vertices into N_guard and N_prison. (could be abstracted).
		for (auto u = l_neighbourhood.begin(); u < l_neighbourhood.end(); ++u) {
			//check if vertex is not a exit_vertex.
			if (std::find(exit_vertices.begin(), exit_vertices.end(), *u) == exit_vertices.end()){
				bool guard_trigger = false;
				//check if a neighbor vertex is adjacent to a exit_vertex.
				auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(*u);
				for (;neigh_itt_u < neigh_itt_u_end; ++neigh_itt_u) {
					//Check if it is a guard_vertex.
					if (std::find(exit_vertices.begin(), exit_vertices.end(), *neigh_itt_u) != exit_vertices.end()) {
						guard_vertices.push_back(*u);
						guard_trigger = true;
						break;
					}
				}
				//Not adjacent to exit_vertex then it is a: prison vertex.
				if (guard_trigger == false) {
					prison_vertices.push_back(*u);
				}
			}
		}

		//find the subset of undominated N_prison vertices.
		std::vector<int>undominated_prison_vertices;
		for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
			if (!mds_context.is_dominated(*i)) {
				undominated_prison_vertices.push_back(*i);
			}
		}
		Logger::execution_time_seperate += t_divide_neighborhood.count();
		//Is there a chance on profit.
		if (!undominated_prison_vertices.empty()) {
			timer t_domination;
			std::vector<std::vector<int>>dominating_subsets;
			//find all combinations.
			const int total_combinations = 1 << l_vertices.size();

			for (int mask = 1; mask < total_combinations; mask++) {
				std::vector<int> combination;
				for (int i = 0; i < l_vertices.size(); ++i){
					if (mask & (1 << i)) {
						combination.push_back(l_vertices[i]);
					}
				}
				//check the combination.
				bool skip_combination = false;  // Flag to track whether to skip the combination

				for (auto& subset : dominating_subsets) {
					//if subset is a (subset) of combination then go further.
					bool is_subset = true;
					for (auto& elem : subset) {
						if (std::find(combination.begin(), combination.end(), elem) == combination.end()) {
							is_subset = false;
							break; // No need to continue checking if one element is missing
						}
					}

					if (is_subset) {
						skip_combination = true;
						break;
					}
				}

				if (skip_combination){
					continue;
				}

				//check if the combination dominates.
				std::unordered_set<int> dominated_vertices;
				mds_context.get_lookup_l_neighborhood(combination, dominated_vertices);
				bool dominates = true;
				for (int undominated_prison_vertex : undominated_prison_vertices){
					if (dominated_vertices.find(undominated_prison_vertex) == dominated_vertices.end()) {
						dominates = false;
						break;
					}
				}
				if (dominates) {
					dominating_subsets.emplace_back(combination);
				}
			}
			Logger::execution_dominations += t_domination.count();

			std::vector<int> W_sizes;
			int num_selector_vertices = 1;
			if (dominating_subsets.size() == 1){
				num_selector_vertices = 0;
			}
			else
			{
				for (auto& constraint : dominating_subsets){
					num_selector_vertices *= constraint.size();
				}
			}

			std::vector<std::pair<std::unordered_set<int>,int>> collection_lookup_dominating_subsets;
			timer t_is_stronger;
			for (auto& w : dominating_subsets) {
				std::unordered_set<int> neighborhood_w;
				mds_context.get_lookup_l_neighborhood(w, neighborhood_w);
				collection_lookup_dominating_subsets.emplace_back(neighborhood_w,w.size());
			}

			// All are stronger.
			std::unordered_set<int>intersection_neighborhood;
			auto [dom, size] = collection_lookup_dominating_subsets.back();
			for (auto elem : dom)
			{
				bool all_present = true;
				for (auto& [domination, s] : collection_lookup_dominating_subsets){
					if (domination.find(elem) == domination.end()) {
						all_present = false;
						break;
					};
				}
				if (all_present) {
					intersection_neighborhood.insert(elem);
				}
			}

			// if (dominating_subsets.size() != 1) {
			// 			return false;
			// 	}

			int total = 0;
			int cnt_undetermined = 0;

			std::vector<int> removable_guard_vertices;
			std::vector<int> removable_prison_vertices;

			for (auto prison : prison_vertices)
			{
				if (intersection_neighborhood.find(prison) == intersection_neighborhood.end()) {
					continue;
				}
				auto [prison_it, prison_it_end] = mds_context.get_neighborhood_itt(prison);
				if (intersection_neighborhood.find(*prison_it) == intersection_neighborhood.end()) {
					continue;
				}
				removable_prison_vertices.push_back(prison);
				total++;
				if (mds_context.is_undetermined(prison))
				{
					cnt_undetermined++;
				}
			}
			for (auto guard : guard_vertices)
			{
				if (intersection_neighborhood.find(guard) == intersection_neighborhood.end()) {
					continue;
				}
				auto [guard_it, guard_it_end] = mds_context.get_neighborhood_itt(guard);
				if (intersection_neighborhood.find(*guard_it) == intersection_neighborhood.end()) {
					continue;
				}
				removable_guard_vertices.push_back(guard);
				total++;
				if (mds_context.is_undetermined(guard))
				{
					cnt_undetermined++;
				}
			}

			if (cnt_undetermined == 0 && dominating_subsets.size() != 1){
				if (total <= num_selector_vertices)
				{
					return false;
				}
			}


			// std::unordered_set<int> all_possibilities;
			// for (auto& set : dominating_subsets) {
			// 	for (auto elem : set) {
			// 		all_possibilities.insert(elem);
			// 	}
			// }
			// if (all_possibilities.size() != l_vertices.size()) {
			// 	return false;
			// }

			timer t_alternative;
			std::unordered_set<int> lookup_n_prison_neighbourhood;
			std::vector<int> n_prison_neighbourhood;
			mds_context.get_l_neighborhood(prison_vertices, lookup_n_prison_neighbourhood, n_prison_neighbourhood);
			for (int prison_vertex : prison_vertices) {
				n_prison_neighbourhood.push_back(prison_vertex);
			}

			//Find all alternative dominations which are smaller than
			// For each size from 1 to max_size
			std::vector<std::vector<int>>alternative_dominations;
			for (int size = 1; size < l_vertices.size(); ++size) {
				// Generate all permutations of the given size
				std::vector<int> combination(size);
				std::vector<bool> chosen(n_prison_neighbourhood.size(), false);

				// Lambda function to generate combinations
				std::function<void(int)> generate = [&](int index) {
					if (index == size) {
						// for (int vertex : combination){
						// 	if (mds_context.is_excluded(vertex)){
						// 		return;
						// 	}
						// }

						// Do something with domination.
						//check the combination.
						bool skip_combination = false;  // Flag to track whether to skip the combination

						for (auto& subset : alternative_dominations) {
							//if subset is a (subset) of combination then go further.
							bool is_subset = true;
							for (auto& elem : subset) {
								if (std::find(combination.begin(), combination.end(), elem) == combination.end()) {
									is_subset = false;
									break; // No need to continue checking if one element is missing
								}
							}

							if (is_subset) {
								skip_combination = true;
								break;
							}
						}

						if (skip_combination){
							return;
						}

						//check if the combination dominates.
						std::unordered_set<int> dominated_vertices;
						mds_context.get_lookup_l_neighborhood(combination, dominated_vertices);
						bool dominates = true;
						for (int undominated_prison_vertex : undominated_prison_vertices){
							if (dominated_vertices.find(undominated_prison_vertex) == dominated_vertices.end()) {
								dominates = false;
								break;
							}
						}
						if (dominates) {
							alternative_dominations.emplace_back(combination);
						}
						return;
					}

					// Try each element in the set to be included in the current combination
					for (int i = 0; i < n_prison_neighbourhood.size(); ++i) {
						if (!chosen[i]) {
							combination[index] = n_prison_neighbourhood[i];
							chosen[i] = true;
							generate(index + 1);  // Recurse to the next position
							chosen[i] = false;    // Unmark the current element (backtrack)
						}
					}
				};

				// Start the recursive generation from index 0
				generate(0);
			}
			Logger::execution_alternative_dominations += t_alternative.count();
			//if for all W in W_alternative, which is better.

			bool is_stronger = true;
			bool subset = false;
			for (auto& w_alter : alternative_dominations) {
				for (auto& [subset_w,w_size] : collection_lookup_dominating_subsets ) {
					subset = false;
					if (w_size <= w_alter.size())
					{
						if (is_superset(mds_context, subset_w, w_alter)){
							subset = true;
							break;
						}
					}
				}
				if (!subset){
					is_stronger = false;
					break;
				}
			}
			 if (!is_stronger){
			 	return false;
			 }

			for (auto prison : removable_prison_vertices){
				mds_context.dominated[prison] = true;
				mds_context.excluded[prison] = true;
				mds_context.removed[prison] = true;
				boost::clear_vertex(prison, mds_context.graph);
			}
			for (auto guard : removable_guard_vertices){
				mds_context.dominated[guard] = true;
				mds_context.excluded[guard] = true;
				mds_context.removed[guard] = true;
				boost::clear_vertex(guard, mds_context.graph);
			}

			if (dominating_subsets.size() == 1){
				Logger::cnt_alber_l_reduction++;
				//std::cout << "new selected vertex found. l_rule" << std::endl;
				//This one can be included.
				for (auto& i : dominating_subsets[0]){
					mds_context.selected[i] = true;
					mds_context.excluded[i] = true;
					mds_context.dominated[i] = true;
					auto [itt, itt_end] = mds_context.get_neighborhood_itt(i);
					for (; itt != itt_end; itt++) {
						mds_context.dominated[*itt] = true;
					}
					mds_context.removed[i] = true;
					boost::clear_vertex(i, mds_context.graph);
				}
				return true;
			} else
			{
				Logger::cnt_alber_l_either_reduction++;
				// Iterate through each unique pair (v1, v2) where v1 != v2
				std::unordered_set<int> all_possibilities;
				for (auto& set : dominating_subsets) {
					for (auto elem : set) {
						all_possibilities.insert(elem);
					}
				}

				std::vector<int> W_sizes;
				int num_selector_vertices = 1;
				for (auto& constraint : dominating_subsets){
					num_selector_vertices *= constraint.size();
					W_sizes.push_back(constraint.size());
				}

				std::vector<vertex>selector_vertices;
				std::vector<std::vector<int>>indices_selector_vertices;
				std::vector<int> current(W_sizes.size(), 0);

				generateSelectors(W_sizes, current, 0, indices_selector_vertices);

				for (int i = 0; i < num_selector_vertices; ++i) {
					auto selector = mds_context.add_vertex();
					selector_vertices.push_back(selector);
				}

				// std::vector<vertex>blocker_vertices;
				// if (all_possibilities.size() < selector_vertices.size()){
				// 	int num_blocker_vertices = selector_vertices.size() - all_possibilities.size();
				// 	for (int q = 0; q < num_blocker_vertices; ++q) {
				// 		auto blocker = mds_context.add_vertex();
				// 		mds_context.exclude_vertex(blocker);
				// 		blocker_vertices.push_back(blocker);
				// 	}
				// }
				for (size_t i = 0; i < dominating_subsets.size(); ++i){
					for (size_t j = 0; j < dominating_subsets[i].size(); ++j){
						for (size_t k = 0; k < selector_vertices.size(); ++k){
							if (indices_selector_vertices[k][i] == j){
								mds_context.add_edge(selector_vertices[k], dominating_subsets[i][j]);
							}
						}

						// for (auto& blocker : blocker_vertices) {
						// 	mds_context.add_edge(blocker, dominating_subsets[i][j]);
						// }
					}
				}
				for (auto& selector : selector_vertices){
						mds_context.excluded[selector] = true;
				}
				Logger::execution_is_stronger += t_is_stronger.count();
				return true;
			}
		}
		return false;
	}

	// Recursive function to generate all selector vectors
	void generateSelectors( const std::vector<int>& W_sizes,std::vector<int>& current, int index, std::vector<std::vector<int>>& selectorVertices) {
		if (index == W_sizes.size()) {
			selectorVertices.push_back(current);
			return;
		}

		for (int i = 0; i < W_sizes[index]; ++i) {
			current[index] = i;
			generateSelectors(W_sizes, current, index + 1, selectorVertices);
		}
	}

	bool is_superset(MDS_CONTEXT& mds_context, std::unordered_set<int>& subset_w, std::vector<int>& w_alter) {
		for (vertex v : w_alter){
			if (subset_w.find(v) == subset_w.end()){
				return false;
			}
			auto [it, it_end] = mds_context.get_neighborhood_itt(v);
			for (; it != it_end; ++it) {
				if (subset_w.find(*it) == subset_w.end()){
					return false;
				}
			}
		}
		return true;
	}
}
