#pragma once
#include "graph_io.h"
#include "context.h"
#include <vector>

MDS_CONTEXT::MDS_CONTEXT(adjacencyListBoost& g) {
	graph = g;
	num_nodes = boost::num_vertices(g);

	included = std::vector<int>(num_nodes, 0);
	dominated = std::vector<int>(num_nodes, 0);
	removed = std::vector<int>(num_nodes, 0);
	update_vertices();

	cnt_sol = 0;
	cnt_dom = 0;
	cnt_rem_v = 0;
	cnt_rem_e = 0;
}

void MDS_CONTEXT::update_vertices() {
	vertices.clear(); //start over
	for (int i = 0; i < num_nodes; ++i) {
		if (removed[i] == 0) {
			vertex v = boost::vertex(i, graph);
			vertices.push_back(v);
		}
	}
}

std::pair<std::vector<vertex>::iterator, std::vector<vertex>::iterator> MDS_CONTEXT::get_vertices_itt() {
	update_vertices();
	return std::make_pair(vertices.begin(), vertices.end());
}

std::pair<std::vector<vertex>::iterator, std::vector<vertex>::iterator> MDS_CONTEXT::get_pair_neighborhood_itt(vertex v, vertex w) {
	
	

}

std::vector<vertex> MDS_CONTEXT::get_pair_neighborhood(vertex v, vertex w) {
	std::vector<vertex>pair_neighborhood_vector;
	std::vector<int> lookup = std::vector<int>(num_nodes, 0);
	auto [vertex_v_itt, vertex_v_itt_end] = boost::adjacent_vertices(v, graph);
	auto [vertex_w_itt, vertex_w_itt_end] = boost::adjacent_vertices(w, graph);
	//add all adjacent vertices of v
	lookup[v] = 1;
	for (;vertex_v_itt < vertex_v_itt_end; ++vertex_v_itt) {
		if (lookup[*vertex_v_itt] == 0) { // we dont want duplicates in our pair_neighborhood_vector
			lookup[*vertex_v_itt] = 1;
			pair_neighborhood_vector.push_back(*vertex_v_itt);
		}
	}
	//add all adjacent vertices of w
	lookup[w] = 1;
	for (;vertex_w_itt < vertex_w_itt_end; ++vertex_w_itt) {
		if (lookup[*vertex_v_itt] == 0) { // we dont want duplicates in our pair_neighborhood_vector
			lookup[*vertex_v_itt] = 1;
			pair_neighborhood_vector.push_back(*vertex_v_itt);
		}
	}

}

adjacencyListBoost& MDS_CONTEXT::get_graph() {
	return(graph);
}

int MDS_CONTEXT::get_total_vertices() {
	return(boost::num_vertices(graph));
}

void MDS_CONTEXT::remove_vertex(vertex v) {
	removed[v] = 1;
	int removed_edges = boost::out_degree(v, graph);
	cnt_rem_e += removed_edges;
	boost::clear_vertex(v, graph);
	cnt_rem_v++;
	num_nodes--;
}

void MDS_CONTEXT::include_vertex(vertex v) {
	included[v] = 1;
	cnt_sol++; //TODO: if you remove an dominated vertex, make the counter go lower.
}

void MDS_CONTEXT::dominate_vertex(vertex v) {
	dominated[v] = 1;
	cnt_dom++;
}

std::pair<adjacency_itt, adjacency_itt> MDS_CONTEXT::get_neighborhood_itt(vertex v) {
	return (boost::adjacent_vertices(v, graph));
}

bool MDS_CONTEXT::is_removed(vertex v) {
	if (removed[v] == 1) {
		return true;
	}
	else {
		return false;
	}
}



