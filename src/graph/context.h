#pragma once

#include "graph_io.h"  // Ensure this contains adjacencyListBoost definition

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;
typedef boost::graph_traits<adjacencyListBoost>::adjacency_iterator adjacency_itt;
typedef boost::graph_traits<adjacencyListBoost>::vertex_iterator vertex_itt;

class MDS_CONTEXT {
    public:
    // Member variables
    std::vector<int> included;
    std::vector<int> dominated;
    std::vector<int> removed;
    std::vector<vertex> vertices;

    adjacencyListBoost graph;
    int num_nodes;

    int cnt_sol;   // Size of the current solution.
    int cnt_dom;   // Number of dominated vertices.
    int cnt_rem_v; // Number of removed vertices.
    int cnt_rem_e; // Number of removed edges.

    // Constructor
    MDS_CONTEXT(adjacencyListBoost& g);

    adjacencyListBoost& get_graph();

    void update_vertices();

    std::pair<std::vector<vertex>::iterator, std::vector<vertex>::iterator> get_vertices_itt();

    int get_total_vertices();

    vector<vertex> get_dominated_vertices();

    void remove_vertex(vertex v);

    void include_vertex(vertex v);

    void dominate_vertex(vertex v);

    bool is_removed(vertex v);

    int get_out_degree_vertex(vertex v);

    std::pair<adjacency_itt, adjacency_itt> get_neighborhood_itt(vertex v);

    std::pair<std::vector<int>, std::vector<vertex>> get_pair_neighborhood(vertex v, vertex w);

    std::vector<vertex> get_undominated_vector(std::vector<vertex>& vertices);

    bool can_be_reduced(std::vector<int>& prison_vertices);

    vertex add_vertex();

    void remove_edge();

    bool is_ignored(vertex v);

    void add_edge(vertex v, vertex w);
};
