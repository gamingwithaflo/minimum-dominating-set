#pragma once
#include "reduce.h"
#include "graph/graph_io.h"
#include "graph/context.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>


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
		//TODO: check whether the vertex is removed in previous reductions.

		//get adjacencyList (itteratable)
		adjacencyListBoost& test = mds_context.get_graph();
		int num_vertices = mds_context.get_num_nodes();
		auto [neigh_itt, neigh_itt_end] = boost::adjacent_vertices(u, test);

		//create adjacencyLookup table 
		std::vector<int>lookup = std::vector<int>(num_vertices, 0);
		for (auto itt = neigh_itt; itt < neigh_itt_end; ++itt) {
			lookup[*itt] = 1;
		}
		// partition neighborhood u into 3 sets.
		std::vector<int>exit_vertices; //N_{3}
		std::vector<int>guard_vertices; //N_{2}
		std::vector<int>prison_vertices; //N_{1}

		//TODO: Identify exit_vertices
		
		//TODO: Identify if remaining vertices go into guard_vertices or prisonn_vertices.
	}
}