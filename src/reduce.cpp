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
		adjacencyListBoost& test = mds_context.get_graph();
		auto [vert_itt, vert_itt_end] = boost::vertices(test);

		for (; vert_itt < vert_itt_end; ++vert_itt) {
			reduce_neighborhood_single_vertex(mds_context, *vert_itt);
		}

	}
	void reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u) {
		//check whether the vertex is removed in previous reductions.
		if (mds_context.is_removed((int)u)) {
			return;
		}

		//get adjacencyList (itteratable)
		adjacencyListBoost& graph = mds_context.get_graph();
		int num_vertices = mds_context.get_num_nodes();
		auto [neigh_itt_u, neigh_itt_u_end] = boost::adjacent_vertices(u, graph);

		//create adjacencyLookup table 
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
			auto [neigh_itt_v, neigh_itt_v_end] = boost::adjacent_vertices(*v, graph);
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
				auto [neigh_itt_v, neigh_itt_v_end] = boost::adjacent_vertices(*v, graph);
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
		if (prison_vertices.size() > 0) {
			mds_context.include_vertex((int)u);
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

			//Remove all guard vertices for complete graph.
		}
	}
}