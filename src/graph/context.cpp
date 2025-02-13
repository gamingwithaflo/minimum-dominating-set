#pragma once
#include "context.h"
#include <vector>

MDS_CONTEXT::MDS_CONTEXT(adjacencyListBoost& g) {
	graph = g;
	num_nodes = boost::num_vertices(g);

	included = std::vector<int>(num_nodes, 0);
	dominated = std::vector<int>(num_nodes, 0);
	removed = std::vector<int>(num_nodes, 0);
	ignored = std::vector<int>(num_nodes, 0); //must be dominated, cannot be chosen.
	update_vertices();

	cnt_sol = 0;
	cnt_dom = 0;
	cnt_rem_v = 0;
	cnt_rem_e = 0;
}

void MDS_CONTEXT::update_vertices() {
	vertices.clear(); //start over
	for (int i = 0; i < boost::num_vertices(graph); ++i) {
		if (removed[i] == 0 && ignored[i] == 0) {
			vertex v = boost::vertex(i, graph);
			vertices.push_back(v);
		}
	}
}

std::pair<std::vector<vertex>::iterator, std::vector<vertex>::iterator> MDS_CONTEXT::get_vertices_itt() {
	update_vertices();
	return std::make_pair(vertices.begin(), vertices.end());
}

std::pair<std::vector<int>,std::vector<vertex>> MDS_CONTEXT::get_pair_neighborhood(vertex v, vertex w) {
	std::vector<vertex>pair_neighborhood_vector;
	std::vector<int> lookup = std::vector<int>(get_total_vertices(), 0);
	auto [vertex_v_itt, vertex_v_itt_end] = boost::adjacent_vertices(v, graph);
	auto [vertex_w_itt, vertex_w_itt_end] = boost::adjacent_vertices(w, graph);
	lookup[v] = 1;
	lookup[w] = 1;
	//add all adjacent vertices of v
	for (;vertex_v_itt < vertex_v_itt_end; ++vertex_v_itt) {
		if (lookup[*vertex_v_itt] == 0) { // we dont want duplicates in our pair_neighborhood_vector
			lookup[*vertex_v_itt] = 1;
			pair_neighborhood_vector.push_back(*vertex_v_itt);
		}
	}
	//add all adjacent vertices of w
	for (;vertex_w_itt < vertex_w_itt_end; ++vertex_w_itt) {
		if (lookup[*vertex_w_itt] == 0) { // we dont want duplicates in our pair_neighborhood_vector
			lookup[*vertex_w_itt] = 1;
			pair_neighborhood_vector.push_back(*vertex_w_itt);
		}
	}
	return(std::make_pair(lookup, pair_neighborhood_vector));
}

//Of a set of vertices, get the subset which is undominated.
std::vector<vertex> MDS_CONTEXT::get_undominated_vector(std::vector<vertex>& vertices) {
	std::vector<vertex> undominated_vector;
	for (auto i = vertices.begin(); i < vertices.end(); ++i) {
		if (dominated[*i] == 0) {
			undominated_vector.push_back(*i);
		}
	}
	return undominated_vector;
}

adjacencyListBoost& MDS_CONTEXT::get_graph() {
	return(graph);
}

int MDS_CONTEXT::get_total_vertices() {
	return(boost::num_vertices(graph));
}

vertex MDS_CONTEXT::get_vertex_from_index(int index) {
	vertex v = boost::vertex(index, graph);
	return v;
}

//update vertices before.
std::vector<vertex> MDS_CONTEXT::get_dominated_vertices() {
	std::vector<vertex> dominated_vertices;
	for (auto i = vertices.begin(); i < vertices.end(); ++i) {
		if (dominated[*i] == 1) {
			dominated_vertices.push_back(*i);
		}
	}
	return dominated_vertices;
}

void MDS_CONTEXT::remove_vertex(vertex v) {
	//keep track in own list which vertices not to consider anymore.
	dominate_vertex(v);
	removed[v] = 1;
	int removed_edges = boost::out_degree(v, graph);
	cnt_rem_e += removed_edges;
	//remove all edges going out of v. (So you do not consider unnessecary vertices).
	boost::clear_vertex(v, graph);
	cnt_rem_v++;
	num_nodes--;
}

void MDS_CONTEXT::include_vertex(vertex v) {
	included[v] = 1;
	cnt_sol++; 
	remove_vertex(v); //Also dominates it for you.
}

void MDS_CONTEXT::dominate_vertex(vertex v) {
	if (dominated[v] == 0) {
		cnt_dom++;
		dominated[v] = 1;
	}
}

std::pair<adjacency_itt, adjacency_itt> MDS_CONTEXT::get_neighborhood_itt(vertex v) {
	return (boost::adjacent_vertices(v, graph));
}

std::pair<edge_itt, edge_itt> MDS_CONTEXT::get_edge_itt() {
	return (boost::edges(graph));
}

int MDS_CONTEXT::get_total_edges() {
	return(boost::num_edges(graph));
}

int MDS_CONTEXT::get_out_degree_vertex(vertex v) {
	int degree = boost::out_degree(v, graph);
	return degree;
}

void MDS_CONTEXT::remove_edge(vertex v, vertex w) {
	boost::remove_edge(v, w, graph);
	cnt_rem_e++;
}

//Check whether the reduction will provide profit.
bool MDS_CONTEXT::can_be_reduced(std::vector<int>& prison_vertices) {
	for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
		//you need at least 1.
		if (dominated[*i] == 0 && removed[*i] == 0) {
			return true;
		}
	}
	//If all prison vertices are dominated, this reduction will not guarentee profit.
	return false;
}

bool MDS_CONTEXT::is_removed(vertex v) {
	if (removed[v] == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool MDS_CONTEXT::edge_exists(vertex v, vertex w) {
	auto [edge,exists] = boost::edge(v, w, graph);
	return exists;
}

bool MDS_CONTEXT::is_dominated(vertex v) {
	if (dominated[v] == 1) {
		return true;
	}
	return false;
}


bool MDS_CONTEXT::is_ignored(vertex v) {
	if (ignored[v] == 1) {
		return true;
	}
	else {
		return false;
	}
}

void MDS_CONTEXT::add_edge(vertex v, vertex w) {
	auto new_edge = boost::add_edge(v, w, graph);
	return;
}

std::pair<std::vector<int>, std::map<int,int>> MDS_CONTEXT::get_undetermined_vertices() {
	std::vector<int>undetermined;
	int total_vertices = get_total_vertices();
	int index = 0;
	std::map<int, int> translation_pace_to_ilp;
	//std::map<int, int> translation_ilp_to_pace; TODO later
	for (int i = 0; i < total_vertices; ++i) {
		if (ignored[i] == 0 && removed[i] == 0) {
			translation_pace_to_ilp[i] = index;
			undetermined.push_back(i);
			index++;
		}
	}
	return (std::make_pair(undetermined, translation_pace_to_ilp));
}

bool MDS_CONTEXT::is_undetermined(vertex v) {
	if (ignored[v] == 0 && removed[v] == 0) {
		return true;
	}
	return false;
}

vertex MDS_CONTEXT::get_source_edge(edge e) {
	return(boost::source(e, graph));
}

vertex MDS_CONTEXT::get_target_edge(edge e) {
	return(boost::target(e, graph));
}

vertex MDS_CONTEXT::add_vertex(){
	vertex new_vertex = boost::add_vertex(graph);
	//assumption new vertex id is just 1 higher.
	included.push_back(0);
	dominated.push_back(0);
	removed.push_back(0);
	ignored.push_back(1);
	num_nodes++;
	return new_vertex;
}



