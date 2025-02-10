#pragma once
#include "reduce.h"
#include "graph/graph_io.h"
#include "graph/context.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>


namespace reduce {
	
	void reduce_graph(MDS_CONTEXT& mds_context) {
		//Get itterator for the vertices.
		int cnt_reductions;
		do {
			cnt_reductions = 0;
			auto [vert_itt, vert_itt_end] = mds_context.get_vertices_itt();

			//TEST
			for (auto itt = vert_itt; itt < vert_itt_end -1; ++itt) {
				auto next_vertex = itt + 1;
				if (reduce_neighborhood_pair_vertices(mds_context, *itt, *next_vertex)) {
					++cnt_reductions;
				}
			}

			for (auto itt = vert_itt; itt < vert_itt_end; ++itt) {
				if (reduce_neighborhood_single_vertex(mds_context, *itt)) {
					++cnt_reductions;
				}
			}
		} while (cnt_reductions > 0);
	}

	bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u) {
		//check whether the vertex is removed in previous reductions.
		if (mds_context.is_removed(u)) {
   			return false;
		}

		//get adjacencyList (itteratable)
		auto [neigh_itt_u, neigh_itt_u_end] = mds_context.get_neighborhood_itt(u);

		//create adjacencyLookup table
		int num_vertices = mds_context.get_total_vertices();
		std::vector<int>lookup = std::vector<int>(num_vertices, 0);
		lookup[u] = 1;
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			lookup[*v] = 1;
		}
		// partition neighborhood u into 3 sets. (COULD: maybe reserve some memory?)
		std::vector<int>exit_vertices; //N_{3}
		std::vector<int>guard_vertices; //N_{2}
		std::vector<int>prison_vertices; //N_{1}

		//Identify exit_vertices
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//for each vertex v get the neighborhood
			auto [neigh_itt_v, neigh_itt_v_end] = mds_context.get_neighborhood_itt(*v);
			//if ANY neighbor isn't in lookup (it belongs to exit_vertices).
			for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				if (lookup[*neigh_itt_v] == 0) {
					exit_vertices.push_back(*v);
					break;
				}
			}
		}
		//Identify if remaining vertices go into guard_vertices or prisonn_vertices.
		for (auto v = neigh_itt_u; v < neigh_itt_u_end; ++v) {
			//check if vertex is not a exit_vertex.
			if (std::find(exit_vertices.begin(), exit_vertices.end(), *v) != exit_vertices.end()) {
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
				mds_context.include_vertex(u);
				mds_context.remove_vertex(u);
				//Remove all Prison vertices for complete graph.
				for (auto itt = prison_vertices.begin(); itt < prison_vertices.end(); ++itt) {
					mds_context.remove_vertex(*itt);
				}
				for (auto itt = guard_vertices.begin(); itt < guard_vertices.end(); ++itt) {
					mds_context.remove_vertex(*itt);
				}
				for (auto itt = exit_vertices.begin(); itt < exit_vertices.end(); ++itt) {
					mds_context.dominate_vertex(*itt);
				}
				return true;
		}
		return false;
	}
	bool reduce_neighborhood_pair_vertices(MDS_CONTEXT& mds_context, vertex v, vertex w) {
		{
			if (mds_context.is_removed(v) || mds_context.is_removed(w)) {
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
					int size = undominated_prison_vertices.size();
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
					//the optimal is either to choose v, w (or both)

					//Create gadget which forces either v, w or both.
					vertex z1 = mds_context.add_vertex();
					vertex z2 = mds_context.add_vertex();
					vertex z3 = mds_context.add_vertex();

					mds_context.add_edge(v, z1);
					mds_context.add_edge(w, z1);
					mds_context.add_edge(v, z2);
					mds_context.add_edge(w, z2);
					mds_context.add_edge(v, z3);
					mds_context.add_edge(w, z3);

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
					mds_context.remove_vertex(v);
					return true;
				}
				if (dominated_by_w) {
					// the optimal is to choose w.
					
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
					mds_context.remove_vertex(w);
					return true;
				}
				//the optimal is to choose both v & w.
				mds_context.include_vertex(v);
				mds_context.remove_vertex(v);

				mds_context.include_vertex(w);
				mds_context.remove_vertex(w);

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

	bool simple_rule_one(vertex v) {
		auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v);
		bool reduced = false;
		for (;neigh_v_itt < neigh_v_itt_end; ++neigh_v_itt) {
			if (is_dominated(*neigh_v_itt)) {
				mds_context.remove_edge(v, *neigh_v_itt);
				reduced = true;
			}
		}
		return reduced;
	}

	bool simple_rule_two(vertex v) {
		if (mds_context.is_removed(v) && mds_context.is_ignored(v)) {
			return false;
		}
		if (mds_context.get_out_degree_vertex(v) <= 1) {
			mds_context.remove_vertex(v);
			return true;
		}
	}

	bool simple_rule_three(vertex v) {
		if (mds_context.is_removed(v) && mds_context.is_ignored(v)) {
			return false;
		}
		if (mds_context.get_out_degree_vertex(v) == 2) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			//rule 3.1
			auto [edge, exists] = boost::edge(u_one, u_two);
			if (exists) {
				mds_context.remove_vertex(v);
				return true;
			}
			//rule 3.2
			auto [neigh_u_one_itt, neigh_u_one_itt_end] = mds_context.get_neighborhood_itt(u_one);
			auto [neigh_u_two_itt, neigh_u_two_itt_end] = mds_context.get_neighborhood_itt(u_one);
			//Sort
			std::sort(neigh_u_one_itt, neigh_u_one_itt_end);
			std::sort(neigh_u_two_itt, neigh_u_two_itt_end);
			//Is there a dist. 3 path from u_one to u_two (besides through v)
			while (neigh_u_one_itt != neigh_u_one_itt_end && neigh_u_two_itt != neigh_u_two_itt_end) {
				if (*neigh_u_one_itt == *neigh_u_two_itt && *neigh_u_one_itt != v) {
					//intersection found
					mds_context.remove_vertex(v);
				}
				else if (*neigh_u_one_itt < *neigh_u_two_itt) {
					++neigh_u_one_itt;
				}
				else {
					++neigh_u_two_itt;
				}
			}
			//no intersection found.
			return false;
		}
		return false;
	}
	bool simple_rule_four(vertex v) {
		if (mds_context.is_removed(v) && mds_context.is_ignored(v)) {
			return false;
		}
		if (mds_context.get_out_degree_vertex(v) == 3) {
			auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v); //This should be 2 vertices.
			vertex u_one = *neigh_v_itt;
			vertex u_two = *++neigh_v_itt;
			vertex u_three = *++neigh_v_itt;
			//rule 3.1
			auto [edge, exists] = boost::edge(u_one, u_two);
			auto [edge_2, exists_2] = boost::edge(u_two, u_three);
			if (exists && exists_2) {
				mds_context.remove_vertex(v);
				return true;
			}
}