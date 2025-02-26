#pragma once
#include "reduce.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>


namespace reduce {
	

	void refractored_reduce_graph(MDS_CONTEXT& mds_context) {
		auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();
		bool first_time = true;
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
						++cnt_reductions;
					}
					if (simple_rule_two(mds_context, *vertex)) {
						++cnt_reductions;
					}
					if (simple_rule_three(mds_context, *vertex)) {
						++cnt_reductions;
					}
					if (simple_rule_four(mds_context, *vertex)) {
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
		} while (cnt_reductions > 0);
	}

	void reduce_ijcai(MDS_CONTEXT& mds_context) {
		bool reduced;
		auto [vertex_itt, vertex_itt_end] = mds_context.get_vertices_itt();

		do {
			reduced = false;

			for (auto itt = vertex_itt; itt < vertex_itt_end; ++itt) {
				if (mds_context.is_undetermined(*itt)) {
					reduced |= reduce_subset(mds_context,*itt);
				}
				if (!mds_context.is_dominated(*itt)) {
					reduced |= reduce_single_dominator(mds_context, *itt);
				}
				if (!mds_context.is_dominated(*itt)) {
					reduced |= reduce_ignore(mds_context, *itt);
				}
			}
		} while (reduced);
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
			mds_context.select_vertex(u);
			++Logger::cnt_reduce_neighborhood_single_vertex;
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
		}
		else if (guard_vertices.size() > 0) {
			for (auto itt : guard_vertices) {
				if (!mds_context.is_excluded(itt)) {
					mds_context.exclude_vertex(itt);
					is_reduced = true;
				}
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
					mds_context.select_vertex(v);
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
					mds_context.select_vertex(w);
					return true;
				}
				//the optimal is to choose both v & w.
				++Logger::cnt_reduce_neighborhood_pair_vertex_both;
				mds_context.select_vertex(v);

				mds_context.select_vertex(w);

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

		if (mds_context.get_out_degree_vertex(v) <= 1) {
			mds_context.remove_vertex(v);
			return true;
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
			if (mds_context.is_undetermined(u_one) || mds_context.is_undetermined(u_two)) {
				return false;
			}
			//the rule is for back black neighboring vertices. (should already not be possible after simple rule 1 but if that one is not on.
			if (mds_context.is_dominated(u_one) || mds_context.is_dominated(u_two) ) {
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
				mds_context.remove_vertex(v);
				return true;
			}
			return false;
		}
		return false;
	}

	//Helper function: does the closed neighborhood of v, contain u. 
	// TODO: if we have a adjacencyMatrix could be done much faster.
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
			if (!mds_context.is_dominated(*itt) && (mds_context.get_frequency(*itt) < minimum_frequency)) {
				minimum_frequency_vertex = *itt;
				minimum_frequency = mds_context.get_frequency(*itt);
			}
		}
		//to make it a closed neighborhood.
		if (!mds_context.is_dominated(v) && (mds_context.get_frequency(v) < minimum_frequency)) {
			minimum_frequency_vertex = v;
			minimum_frequency = mds_context.get_frequency(v);
		}

		//find vertices which needs to be dominated.
		std::vector<vertex> needs_to_dominate;
		if (!mds_context.is_dominated(v) && v != minimum_frequency_vertex) {
			needs_to_dominate.push_back(v);
		}
		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt) {
			if (minimum_frequency_vertex != *itt && !mds_context.is_dominated(*itt)) {
				needs_to_dominate.push_back(*itt);
			}
		}

		//get all possible vertices which could be the superset.
		auto [neigh_itt_mf, neigh_itt_mf_end] = mds_context.get_neighborhood_itt(minimum_frequency_vertex);
		for (auto itt = neigh_itt_mf; itt < neigh_itt_mf_end; ++itt) {
			if (*itt == v || mds_context.is_dominated(*itt) || mds_context.get_frequency(*itt) < mds_context.get_frequency(v)) {
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
		if (minimum_frequency_vertex == v || mds_context.is_dominated(minimum_frequency_vertex) || minimum_frequency < mds_context.get_frequency(v)) {
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
			if (*itt == v || mds_context.is_dominated(*itt) || mds_context.get_frequency(*itt) < mds_context.get_frequency(v)) {
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
		if (minimum_coverage_vertex == v || mds_context.is_dominated(minimum_coverage_vertex) || mds_context.get_frequency(minimum_coverage_vertex) < mds_context.get_frequency(v)) {
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
}