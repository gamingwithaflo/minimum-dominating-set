#pragma once
#include "reduce.h"
#include "graph/context.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>

#include "util/logger.h"


namespace reduce {
	
	void log_reduce_graph(MDS_CONTEXT& mds_context) {
		bool first_time = true;
		int cnt_reductions;
		do {
			cnt_reductions = 0;
			auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
			std::vector<vertex>dominated_vertices = mds_context.get_dominated_vertices(); //vertices list get updated in mds_context object by get_vertices_itt.
			//these rules currently are only applied to undetermined, dominated vertices.
			for (auto itt = dominated_vertices.begin(); itt < dominated_vertices.end(); ++itt) {
				if (dominated_subset_rule(mds_context, *itt)) {
					++cnt_reductions;
				}
				if (Logger::flag_sr_1) {
					++Logger::att_simple_rule_one;
					if (simple_rule_one(mds_context, *itt)) {
						++cnt_reductions;
						++Logger::cnt_simple_rule_one;
					}
				}
				if (Logger::flag_sr_2) {
					++Logger::att_simple_rule_two;
					if (simple_rule_two(mds_context, *itt)) {
						++cnt_reductions;
						++Logger::cnt_simple_rule_two;
					}
				}
				if (Logger::flag_sr_3) {
					++Logger::att_simple_rule_three;
					if (simple_rule_three(mds_context, *itt)) {
						++cnt_reductions;
					}
				}
				if (Logger::flag_sr_4) {
					++Logger::att_simple_rule_four;
					if (simple_rule_four(mds_context, *itt)) {
						++cnt_reductions;
						++Logger::cnt_simple_rule_four;
					}
				}
			}
			if (Logger::flag_neigh_single) {
				for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
					++Logger::att_reduce_neighborhood_single_vertex;
					if (reduce_neighborhood_single_vertex(mds_context, *itt)) {
						++cnt_reductions;
					}
				}
			}
			if (cnt_reductions == 0 && first_time && Logger::flag_neigh_pair) {
				for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
					//distance vector.
					std::vector<int>distance (mds_context.get_total_vertices(), -1);
					distance[*itt] = 0; //source.
					//fill distance vector (up to dist 3).
					auto [neigh_depth_one, neigh_depth_one_end] = mds_context.get_neighborhood_itt(*itt);
					for (;neigh_depth_one < neigh_depth_one_end; ++neigh_depth_one) {
						if (distance[*neigh_depth_one] != 0) {
							distance[*neigh_depth_one] = 1;
						}
						auto [neigh_depth_two, neigh_depth_two_end] = mds_context.get_neighborhood_itt(*neigh_depth_one);
						for (;neigh_depth_two < neigh_depth_two_end; ++neigh_depth_two) {
							if (distance[*neigh_depth_two] != 0) {
								distance[*neigh_depth_two] = 1;
							}
							auto [neigh_depth_three, neigh_depth_three_end] = mds_context.get_neighborhood_itt(*neigh_depth_two);
							for (;neigh_depth_three < neigh_depth_three_end; ++neigh_depth_three) {
								if (distance[*neigh_depth_three] != 0) {
									distance[*neigh_depth_two] = 1; //only possible untill distance 3.
								}
							}
						}
					}
					for (size_t i = 0; i < distance.size(); ++i) {
						if (distance[i] == 1 && *itt < i) { // you do not want to perform it twice.
							++Logger::att_reduce_neighborhood_pair_vertex;
							if (reduce_neighborhood_pair_vertices(mds_context, *itt, mds_context.get_vertex_from_index(i))) {
								++cnt_reductions;
							}
						}
					}
				}
				first_time = false;
			}
		} while (cnt_reductions > 0);
	}

	void reduce_graph(MDS_CONTEXT& mds_context) {
		//Get itterator for the vertices.
		int cnt_reductions;
		bool first_time = true;

		do {
			cnt_reductions = 0;
			auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
			std::vector<vertex>dominated_vertices = mds_context.get_dominated_vertices();
			for (auto itt = dominated_vertices.begin(); itt < dominated_vertices.end(); ++itt) {
				if (simple_rule_one(mds_context, *itt)) {
					++cnt_reductions;
				}
				if (simple_rule_two(mds_context, *itt)) {
					++cnt_reductions;
				}
				if (simple_rule_two(mds_context, *itt)) {
					++cnt_reductions;
				}
				if (simple_rule_three(mds_context, *itt)) {
					++cnt_reductions;
				}
				if (simple_rule_four(mds_context, *itt)) {
					++cnt_reductions;
				}
			}
			for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
				if (reduce_neighborhood_single_vertex(mds_context, *itt)) {
					++cnt_reductions;
				}
			}
			if (cnt_reductions == 0 && first_time) {
				for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
					//distance vector.
					std::vector<int>distance (mds_context.get_total_vertices(), -1);
					distance[*itt] = 0; //source.
					//fill distance vector (up to dist 3).
					auto [neigh_itt_itt, neigh_itt_itt_end] = mds_context.get_neighborhood_itt(*itt);
					for (;neigh_itt_itt < neigh_itt_itt_end; ++neigh_itt_itt) {
						if (distance[*neigh_itt_itt] != 0) {
							distance[*neigh_itt_itt] = 1;
						}
						auto [neigh_itt_itt_itt, neigh_itt_itt_itt_end] = mds_context.get_neighborhood_itt(*neigh_itt_itt);
						for (;neigh_itt_itt_itt < neigh_itt_itt_itt_end; ++neigh_itt_itt_itt) {
							if (distance[*neigh_itt_itt_itt] != 0) {
								distance[*neigh_itt_itt_itt] = 1;
							}
						}
					}
					for (size_t i = 0; i < distance.size(); ++i) {
						if (distance[i] == 1) {
							if (reduce_neighborhood_pair_vertices(mds_context, *itt, mds_context.get_vertex_from_index(i))) {
								++cnt_reductions;
							}
						}
					}
				}
				first_time = false;
			}
		} while (cnt_reductions > 0);
	}

	

	bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u) {
		//check whether the vertex is removed in previous reductions. (if vertex is excluded there exists an better option so will never be chosen).
		if (mds_context.is_removed(u) || mds_context.is_excluded(u)) {
   			return false;
		}

		bool is_reduced = false;

		//get adjacencyList (itteratable)
		auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(u);

		//create adjacencyLookup table
		int num_vertices = mds_context.get_total_vertices();
		std::vector<int>lookup = std::vector<int>(num_vertices, 0);
		lookup[u] = 1;
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			lookup[*v] = 1;
		}
		// partition neighborhood u into 4 sets
		std::vector<int>non_exit_vertices;
		std::vector<int>exit_vertices; //N_{3}
		std::vector<int>guard_vertices; //N_{2}
		std::vector<int>prison_vertices; //N_{1}

		//Identify exit_vertices
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//for each vertex v get the neighborhood
			auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(*v);
			//if ANY neighbor isn't in lookup (it belongs to exit_vertices).
			bool all_dominated_flag = true;
			bool all_excluded_flag = true;
			bool prison = true;
			for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				if (lookup[*neigh_itt_v] == 0) {
					if (mds_context.is_excluded(*neigh_itt_v)) {
						all_dominated_flag = false;
						prison = false;
						if (all_excluded_flag) {
							continue;
						}
						break;
					}
					if (mds_context.is_dominated(*neigh_itt_v) || mds_context.is_ignored(*neigh_itt_v)) { //should also handle actual excluded.
						all_excluded_flag = false;
						prison = false;
						if (all_dominated_flag) {
							continue;
						}
						break;
					}
					prison = false;
					all_dominated_flag = false;
					all_excluded_flag = false;
					break;
				}
			}
			//combination of excluded vertices and dominated / ignored vertices.
			if (all_excluded_flag == true && prison == false) {
				if (!mds_context.is_ignored(u)) {
 					mds_context.ignore_vertex(u);
					is_reduced = true;
				}
				exit_vertices.push_back(*v);
				continue;
			}
			//If all neighbors are dominated or excluded.
			if (all_dominated_flag == true && prison == false) {
				non_exit_vertices.push_back(*v);
				continue;
			}
			if (prison == false) {
				exit_vertices.push_back(*v);
			}
		}
		//Identify if remaining vertices go into guard_vertices or prison_vertices.
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//check if vertex is not a exit_vertex.
			if (std::find(exit_vertices.begin(), exit_vertices.end(), *v) != exit_vertices.end()) {
				continue;
			}
			//check if vertex is not a non-exit.
			if (std::find(non_exit_vertices.begin(), non_exit_vertices.end(), *v) != non_exit_vertices.end()) {
				continue;
			}
			else {
				bool guard_trigger = false;
				auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(*v);
				for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
					//Check if it is a guard_vertex.
					if (std::find(exit_vertices.begin(), exit_vertices.end(), *neigh_itt_v) != exit_vertices.end()) {
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
		}
		//Check whether the graph can be reduced.
		if (mds_context.can_be_reduced(prison_vertices)) {
			is_reduced = true;
			mds_context.include_vertex(u);
			++Logger::cnt_reduce_neighborhood_single_vertex;
			//Remove all Prison vertices for complete graph.
			for (auto itt = prison_vertices.begin(); itt < prison_vertices.end(); ++itt) {
				mds_context.exclude_vertex(*itt);
				mds_context.remove_vertex(*itt);
			}
			for (auto itt = guard_vertices.begin(); itt < guard_vertices.end(); ++itt) {
				mds_context.exclude_vertex(*itt);
				mds_context.remove_vertex(*itt);
			}
			for (auto itt = non_exit_vertices.begin(); itt < non_exit_vertices.end(); ++itt) {
				mds_context.exclude_vertex(*itt);
				mds_context.remove_vertex(*itt);
			}
			for (auto itt = exit_vertices.begin(); itt < exit_vertices.end(); ++itt) {
				mds_context.dominate_vertex(*itt);
			}
			return is_reduced;
		} else if (guard_vertices.size() > 0) {
			if (!mds_context.is_ignored(u)) {
				mds_context.ignore_vertex(u);
				is_reduced = true;
			}
			for (auto itt = guard_vertices.begin(); itt < guard_vertices.end(); ++itt) {
				if (!mds_context.is_excluded(*itt)) {
					if (mds_context.is_dominated(*itt)) {
						mds_context.remove_vertex(*itt);
					}
					mds_context.exclude_vertex(*itt); //same as exclude. also add
					is_reduced = true;
				}
			}
			if (non_exit_vertices.size() > 0) {
				for (auto itt = non_exit_vertices.begin(); itt < non_exit_vertices.end(); ++itt) {
					if (!mds_context.is_excluded(*itt)) {
						if (mds_context.is_dominated(*itt)) {
							mds_context.remove_vertex(*itt);
						}
						mds_context.exclude_vertex(*itt); //same as exclude. also add
						is_reduced = true;
					}
				}
			}
			if (is_reduced) {
				++Logger::cnt_reduce_neighborhood_single_vertex_guard;
			}
			return is_reduced; // actually just the subset rule.
		}
		else if (non_exit_vertices.size() > 0) {
			if (non_exit_vertices.size() > 0) {
				for (auto itt = non_exit_vertices.begin(); itt < non_exit_vertices.end(); ++itt) {
					if (!mds_context.is_excluded(*itt)) {
						if (mds_context.is_dominated(*itt)) {
							mds_context.remove_vertex(*itt);
						}
						mds_context.exclude_vertex(*itt); //same as exclude. also add
						is_reduced = true;
					}
				}
			}
			if (is_reduced) {
				++Logger::cnt_reduce_neighborhood_single_vertex_guard;
			}
			return is_reduced; // actually just the subset rule.
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

			//Identify exit_vertices (TODO: could be abstracted).
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
					++Logger::cnt_reduce_neighborhood_pair_vertex_either;
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
					return true;
				}
				if (dominated_by_v) {
					// the optimal is to choose v.
					++Logger::cnt_reduce_neighborhood_pair_vertex_single;
					//remove all prison vertices.
					for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
						mds_context.remove_vertex(*i);
					}

					auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
					for (; neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
						//dominate neighborhood of w, because it gets included.
						mds_context.dominate_vertex(*neigh_itt_v);
						if (std::find(guard_vertices.begin(), guard_vertices.end(), *neigh_itt_v) != guard_vertices.end()) {
							//if it is a guard it can be removed.
							mds_context.remove_vertex(*neigh_itt_v);
						}
					}
					mds_context.include_vertex(v);
					return true;
				}
				if (dominated_by_w) {
					// the optimal is to choose w.
					++Logger::cnt_reduce_neighborhood_pair_vertex_single;
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
					mds_context.include_vertex(w);
					return true;
				}
				//the optimal is to choose both v & w.
				++Logger::cnt_reduce_neighborhood_pair_vertex_both;
				mds_context.include_vertex(v);

				mds_context.include_vertex(w);

				//dominate pair_neighborhood
				for (auto u = prison_vertices.begin(); u < prison_vertices.end(); ++u) {
					mds_context.remove_vertex(*u);
				}
				for (auto u = guard_vertices.begin(); u < guard_vertices.end(); ++u) {
					mds_context.remove_vertex(*u);
				}
				for (auto u = exit_vertices.begin(); u < exit_vertices.end(); ++u) {
					mds_context.dominate_vertex(*u);
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
		if (mds_context.is_removed(v) || mds_context.is_ignored(v)) {
			return false;
		}
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
		if (mds_context.is_removed(v) || mds_context.is_ignored(v)) {
			return false;
		}
		if (mds_context.get_out_degree_vertex(v) <= 1) {
			mds_context.remove_vertex(v);
			return true;
		}
		return false;
	}

	bool simple_rule_three(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.is_removed(v) || mds_context.is_ignored(v)) {
			return false;
		}
		//in principle could u1 & u2 not be dominated because all edges between dominated vertices should be removed by a previous rule.
		if (mds_context.get_out_degree_vertex(v) == 2) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			//Not both vertices should be excluded.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			if (mds_context.is_undetermined(u_one) || mds_context.is_undetermined(u_two)) {
				return false;
			}
			//the rule is for back black neighboring vertices. (should already not be possible after simple rule 1 but if that one is not on.
			if (mds_context.is_dominated(u_one) || mds_context.is_dominated(u_two) || mds_context.is_ignored(u_one) || mds_context.is_ignored(u_two)) {
				return false;
			}
			//rule 3.1
			if (mds_context.edge_exists(u_one, u_two)) {
				mds_context.remove_vertex(v);
				++Logger::cnt_simple_rule_three_dot_one;
				return true;
			}
			//rule 3.2
			auto [neigh_u_one_itt, neigh_u_one_itt_end] = mds_context.get_neighborhood_itt(u_one);
			for (;neigh_u_one_itt < neigh_u_one_itt_end; ++neigh_u_one_itt) {
				//u_2 must be not dominated & not excluded (or undetermined) (is allowed to be ignored /dominated)
				if (*neigh_u_one_itt != v && mds_context.edge_exists(*neigh_u_one_itt, u_two) && mds_context.is_undetermined(*neigh_u_one_itt)) { //pretty sure this is a bug.
					mds_context.remove_vertex(v);
					++Logger::cnt_simple_rule_three_dot_two;
					return true;
				}
			}
			//no intersection found.
			return false;
		}
		return false;
	}
	bool simple_rule_four(MDS_CONTEXT& mds_context, vertex v) {
		if (mds_context.is_removed(v) || mds_context.is_ignored(v)) {
			return false;
		}

		if (mds_context.get_out_degree_vertex(v) == 3) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			vertex u_three = *++neigh_v_itt;
			if (mds_context.is_dominated(u_one) || mds_context.is_dominated(u_two) || mds_context.is_ignored(u_one) || mds_context.is_ignored(u_two) || mds_context.is_dominated(u_three) || mds_context.is_ignored(u_three)) {
				return false;
			}
			//rule 4
			bool exists = mds_context.edge_exists(u_one, u_two);
			auto exists_2 = mds_context.edge_exists(u_two, u_three);
			if (exists && exists_2 && mds_context.is_undetermined(u_one) && mds_context.is_undetermined(u_two) && mds_context.is_undetermined(u_three)) {
				mds_context.remove_vertex(v);
				return true;
			}
			return false;
		}
		return false;
	}
	bool dominated_subset_rule(MDS_CONTEXT& mds_context, vertex v) {
		//find all vertices.
		std::vector<vertex> need_to_dominate;

		std::vector<vertex> potential_supersets;

		auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(v);
		need_to_dominate = mds_context.get_frequency(v);
		//get the number of valuable outgoing vertices.
		int frequency_v = need_to_dominate.size();

		// get all neighbors of v.
		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt){
			auto [neigh_itt_depth_2, neigh_itt_depth_2_end] = mds_context.get_neighborhood_itt(*itt);
			//get all vertices of distance 2.
			for (; neigh_itt_depth_2 < neigh_itt_depth_2_end; ++neigh_itt_depth_2) {
				//calculate their coverage / frequency. (could be that excluded vertices do not count).
				std::vector<vertex> coverage;
				coverage = mds_context.get_frequency(*neigh_itt_depth_2);
				int frequency_w = coverage.size();
				//This guarantees that v never gets in potential_supersets.
				if (frequency_w > frequency_v) {
					if (std::find(potential_supersets.begin(), potential_supersets.end(), *neigh_itt_depth_2) != potential_supersets.end()) {
						continue;
					}
					potential_supersets.push_back(*neigh_itt_depth_2);
				}
			}
		}
		//check whether potential_supersets have edges to all need_to_dominate.
		for (auto superset = potential_supersets.begin(); superset < potential_supersets.end(); ++superset) {
			bool all_edges_present = true;
			for (auto dominated = need_to_dominate.begin(); dominated < need_to_dominate.end(); ++dominated) {
				if (!mds_context.edge_exists(*superset, *dominated)) {
					all_edges_present = false;
					break;
				}
			}
			if (all_edges_present) {
				mds_context.exclude_vertex(v);
				mds_context.remove_vertex(v); // v is dominated so you should just be able to remove it.
				return true;
			}
		}
		return false;
	}
}