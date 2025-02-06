#pragma once

#include "graph_io.h"  // Ensure this contains adjacencyListBoost definition

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

class MDS_CONTEXT {
    public:
    // Member variables
    std::vector<int> included;
    std::vector<int> dominated;
    std::vector<int> removed;

    adjacencyListBoost graph;
    int num_nodes;

    int cnt_sol;   // Size of the current solution.
    int cnt_dom;   // Number of dominated vertices.
    int cnt_rem_v; // Number of removed vertices.
    int cnt_rem_e; // Number of removed edges.

    // Constructor
    MDS_CONTEXT(adjacencyListBoost& g);

    adjacencyListBoost& get_graph();

    int get_num_nodes();

    void remove_vertex(vertex v);

    void include_vertex(int index);

    void dominate_vertex(int index);

    bool is_removed(int index);
};
