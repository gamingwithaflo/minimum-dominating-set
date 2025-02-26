#pragma once
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;


typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;
typedef boost::graph_traits<adjacencyListBoost>::edge_descriptor edge;
typedef boost::graph_traits<adjacencyListBoost>::adjacency_iterator adjacency_itt;
typedef boost::graph_traits<adjacencyListBoost>::vertex_iterator vertex_itt;
typedef boost::graph_traits<adjacencyListBoost>::edge_iterator edge_itt;

class MDS_CONTEXT {
public:
    int num_nodes;
    adjacencyListBoost graph;

    std::vector<int> selected;
    std::vector<int> dominated;
    std::vector<int> removed;
    std::vector<int> excluded;
    std::vector<int> ignored;

    std::vector<int> c_d;
    std::vector<int> c_nd;
    std::vector<int> c_x;

    int cnt_sel;   // Size of the current solution.
    int cnt_dom;   // Number of dominated vertices.
    int cnt_excl; // Number of removed vertices.

    // Constructor
    MDS_CONTEXT(adjacencyListBoost& g);

    adjacencyListBoost& get_graph();

    std::pair<vertex_itt, vertex_itt> get_vertices_itt();

    int get_total_vertices();

    int get_total_edges();

    int get_coverage_size(vertex v);

    std::pair<edge_itt, edge_itt> get_edge_itt();

    vertex get_source_edge(edge e);

    vertex get_target_edge(edge e);

    std::pair<std::vector<int>, std::map<int, int>> get_undetermined_vertices();

    void remove_vertex(vertex v);

    void select_vertex(vertex v);

    void dominate_vertex(vertex v);

    bool is_dominated(vertex v);

    bool is_removed(vertex v);

    bool is_selected(vertex v);

    bool is_undetermined(vertex v);

    bool edge_exists(vertex v, vertex w);

    int get_out_degree_vertex(vertex v);

    vertex get_vertex_from_index(int index);

    std::pair<adjacency_itt, adjacency_itt> get_neighborhood_itt(vertex v);

    std::pair<std::vector<int>, std::vector<vertex>> get_pair_neighborhood(vertex v, vertex w);

    std::vector<vertex> get_undominated_vector(std::vector<vertex>& vertices);

    bool can_be_reduced(std::vector<int>& prison_vertices);

    vertex add_vertex();

    void remove_edge(vertex v, vertex w);

    bool is_excluded(vertex v);

    void add_edge(vertex v, vertex w);

    void exclude_vertex(vertex v);

    void ignore_vertex(vertex v);

    int get_frequency(vertex v);

    bool is_ignored(vertex v);
};