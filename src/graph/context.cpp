#pragma once
#include "context.h"
#include <vector>

MDS_CONTEXT::MDS_CONTEXT(adjacencyListBoost& g) {
	graph = g;
	num_nodes = boost::num_vertices(g);

	selected = std::vector<bool>(num_nodes, false);
	dominated = std::vector<bool>(num_nodes, false);
	removed = std::vector<bool>(num_nodes, false);
	excluded = std::vector<bool>(num_nodes, false);
	ignored = std::vector<bool>(num_nodes, false);

	c_d = std::vector<int>(num_nodes, 0);
	c_nd = std::vector<int>(num_nodes, 0);
	c_x = std::vector<int>(num_nodes, 0);

	cnt_sel = 0;   
	cnt_dom = 0;   
	cnt_excl = 0;
	cnt_ign = 0;
	cnt_rem = 0;
}

void MDS_CONTEXT::select_vertex(vertex v) {
	selected[v] = true;
	cnt_sel++;
	auto [neigh_itt_v, neigh_itt_v_end] = get_neighborhood_itt(v);
	dominate_vertex(v);
	for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
		dominate_vertex(*neigh_itt_v);
	}
}

bool MDS_CONTEXT::is_selected(vertex v) {
	return selected[v];
}

void MDS_CONTEXT::dominate_vertex(vertex v) {
	if (!is_dominated(v)) {
		cnt_dom++;
		dominated[v] = true;
		if (!is_ignored(v)) {
			c_nd[v]++;
			auto [neigh_itt_v, neigh_itt_v_end] = get_neighborhood_itt(v);
			for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				c_nd[*neigh_itt_v]++;
			}
		}
	}
}

bool MDS_CONTEXT::is_dominated(vertex v) {
	return dominated[v];
}

bool MDS_CONTEXT::is_dominated_ijcai(vertex v) {
	return (dominated[v]  || ignored[v]);
}

void MDS_CONTEXT::exclude_vertex(vertex v) {
	if (!is_excluded(v)) {
		cnt_excl++;
		excluded[v] = true;
		c_x[v]++;
		auto [neigh_itt_v, neigh_itt_v_end] = get_neighborhood_itt(v);

		if (!is_dominated_ijcai(v) && get_frequency(v) == 1) {
			for (auto itt = neigh_itt_v ;itt < neigh_itt_v_end; ++itt) {
				if (!is_excluded(*itt)) {
					select_vertex(*itt);
					break;
				}
			}
		}
		for (auto itt = neigh_itt_v; itt < neigh_itt_v_end; ++itt) {
			c_x[*itt]++;
			if (!is_dominated_ijcai(*itt) && get_frequency(*itt) == 1) {
				auto [neigh_itt, neigh_itt_end] = get_neighborhood_itt(*itt);
				if (!is_excluded(*itt)) {
					select_vertex(*itt);
				}
				for (;neigh_itt < neigh_itt_end; ++neigh_itt) {
					if (!is_excluded(*neigh_itt)) {
						select_vertex(*neigh_itt);
						break;
					}
				}
			}
		}
	}
}

bool MDS_CONTEXT::is_excluded(vertex v) {
	return excluded[v];
}

void MDS_CONTEXT::ignore_vertex(vertex v) {
	if (!is_ignored(v)) {
		ignored[v] = true;
		cnt_ign++;

		//check if it isn't increased because it is already dominated.
		if (!is_dominated(v)) {
			c_nd[v]++;
			auto [neigh_itt_v, neigh_itt_v_end] = get_neighborhood_itt(v);
			for (;neigh_itt_v < neigh_itt_v_end; ++neigh_itt_v) {
				c_nd[*neigh_itt_v]++;
			}
		}
	}
}

bool MDS_CONTEXT::is_ignored(vertex v) {
	return ignored[v];
}

bool MDS_CONTEXT::is_undetermined(vertex v) {
	if (!is_excluded(v) && !is_selected(v)) {
		return true;
	}
	return false;
}

//select v into solution will dominate |N[v]| - c_nd[u] more vertices.
//coverage: how many more vertices will be dominated if you select this vertex.
int MDS_CONTEXT::get_coverage_size(vertex v) {
	//v must be undetermined.
	int num_closed_neighborhood = get_out_degree_vertex(v) + 1;
	return num_closed_neighborhood - c_nd[v];
}

int MDS_CONTEXT::get_frequency(vertex v) {
	int num_closed_neighborhood = get_out_degree_vertex(v) + 1;
	return num_closed_neighborhood - c_x[v];
}

void MDS_CONTEXT::get_lookup_l_neighborhood(std::vector<int>& l_vertices, std::unordered_set<int>& lookup_neighbourhood){
	for (auto& v : l_vertices) {
		auto [vertex_it, vertex_it_end] = boost::adjacent_vertices(v, graph);
		//insert itself.
		lookup_neighbourhood.emplace(v);

		for (; vertex_it < vertex_it_end; ++vertex_it ) {
			// if neighbour does not exist yet, insert.
			if (lookup_neighbourhood.find(*vertex_it) == lookup_neighbourhood.end()) {
				lookup_neighbourhood.emplace(*vertex_it);
			}
		}
	}
}

void MDS_CONTEXT::get_l_neighborhood(std::vector<int>& l_vertices, std::unordered_set<int>& lookup_neighbourhood, std::vector<int>& l_neighbourhood){
	for (auto& v : l_vertices) {
		lookup_neighbourhood.emplace(v);
	}

	for (auto& v : l_vertices) {
		auto [vertex_it, vertex_it_end] = boost::adjacent_vertices(v, graph);

		for (; vertex_it < vertex_it_end; ++vertex_it ) {
			// if neighbour does not exist yet, insert.
			if (lookup_neighbourhood.find(*vertex_it) == lookup_neighbourhood.end())
			{
				lookup_neighbourhood.emplace(*vertex_it);
				//only check not removed vertices.
				if (!is_selected(*vertex_it) && !(is_excluded(*vertex_it) && is_dominated(*vertex_it))) {
					l_neighbourhood.push_back(*vertex_it);
				}
			}
		}
	}
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
			if (is_dominated(*vertex_v_itt) && (is_selected(*vertex_v_itt))) {
				continue;
			}
			pair_neighborhood_vector.push_back(*vertex_v_itt);
		}
	}
	//add all adjacent vertices of w
	for (;vertex_w_itt < vertex_w_itt_end; ++vertex_w_itt) {
		if (lookup[*vertex_w_itt] == 0) { // we dont want duplicates in our pair_neighborhood_vector
			lookup[*vertex_w_itt] = 1;
			if (is_dominated(*vertex_w_itt) && (is_selected(*vertex_w_itt))) {
				continue;
			}
			pair_neighborhood_vector.push_back(*vertex_w_itt);
		}
	}
	return(std::make_pair(lookup, pair_neighborhood_vector));
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

std::pair<vertex_itt, vertex_itt>  MDS_CONTEXT::get_vertices_itt() {
	return (boost::vertices(graph));
}

std::vector<vertex> MDS_CONTEXT::get_vertices() {
	std::vector<vertex> vertices;
	auto [vert_itt, vert_itt_end] = get_vertices_itt();
	for (;vert_itt < vert_itt_end; ++vert_itt) {
		if (is_undetermined(*vert_itt)) {
			vertices.push_back(*vert_itt);
		}
	}
	return vertices;
}

void MDS_CONTEXT::fill_removed_vertex() {
	for (int counter = 0; counter < removed.size(); ++counter) {
		if (is_dominated(counter) && is_excluded(counter)) {
			removed[counter] = true;
			cnt_rem++;
		}
	}
}

void MDS_CONTEXT::remove_vertex(vertex v) {
	//keep track in own list which vertices not to consider anymore.
	removed[v] = true;
	dominate_vertex(v);
	if (!is_selected(v)) {
		exclude_vertex(v);
	}
	
	int removed_edges = boost::out_degree(v, graph);
	//remove all edges going out of v. (So you do not consider unnessecary vertices).
	boost::clear_vertex(v, graph);
	num_nodes--;
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
}

//Check whether the reduction will provide profit.
bool MDS_CONTEXT::can_be_reduced(std::vector<int>& prison_vertices) {
	for (auto i = prison_vertices.begin(); i < prison_vertices.end(); ++i) {
		//you need at least 1.
		if (!dominated[*i] && !removed[*i]) {
			return true;
		}
	}
	//If all prison vertices are dominated, this reduction will not guarentee profit.
	return false;
}

bool MDS_CONTEXT::is_removed(vertex v) {
	if (removed[v]) {
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
		if (!excluded[i] && !removed[i] && !selected[i]) {
			translation_pace_to_ilp[i] = index;
			undetermined.push_back(i);
			index++;
		}
	}
	return (std::make_pair(undetermined, translation_pace_to_ilp));
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
	selected.push_back(false);
	dominated.push_back(false);
	removed.push_back(false);
	excluded.push_back(false);
	ignored.push_back(false);
	c_d.push_back(0);
	c_nd.push_back(0);
	c_x.push_back(0);

	num_nodes++;
	return new_vertex;
}

void MDS_CONTEXT::fill_mds_context(MDS_CONTEXT& mds_context, std::unordered_map<int, int>& newToOldIndex) {
	int num_vertices = get_total_vertices();
	for (int i = 0; i < num_vertices; ++i) {
		int OldIndex = newToOldIndex[i];
		if (mds_context.is_dominated(OldIndex)) {
			dominated[i] = true;
		}
		if (mds_context.is_excluded(OldIndex)) {
			excluded[i] = true;
		}
		// if (mds_context.is_ignored(OldIndex)) {
		// 	ignore_vertex(i);
		// }
	}
}



