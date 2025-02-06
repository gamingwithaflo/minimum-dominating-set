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

	cnt_sol = 0;
	cnt_dom = 0;
	cnt_rem_v = 0;
	cnt_rem_e = 0;
}

adjacencyListBoost& MDS_CONTEXT::get_graph() {
	return(graph);
}

int MDS_CONTEXT::get_num_nodes() {
	return(num_nodes);
}

void MDS_CONTEXT::remove_vertex(vertex v) {
	removed[v] = 1;
	boost::clear_vertex(v, graph);
}

void MDS_CONTEXT::include_vertex(int index) {
	included[index] = 1;
}

void MDS_CONTEXT::dominate_vertex(int index) {
	dominated[index] = 1;
}

bool MDS_CONTEXT::is_removed(int index) {
	if (removed[index] == 1) {
		return (true);
	}
	else {
		return (false);
	}
}



