#pragma once

#include <vector>
#include <boost/graph/adjacency_list.hpp>

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

enum operation_enum {
    FORGET,
    JOIN,
    INTRODUCE,
    INTRODUCE_EDGE,
    LEAF
};

struct operation {
    operation_enum opp;

    //constructor
    operation(operation_enum type);
    //destructor
    virtual ~operation() = default;
};

struct operation_join : operation {

    //constructor
    operation_join();
};

struct operation_leaf : operation {

    //constructor
    operation_leaf();
};

struct operation_forget : operation {
    int vertex;

    //constructor
    operation_forget(int v);
};


struct operation_introduce : operation {
    int vertex;

    //constructor
    operation_introduce(int v);
};

struct operation_introduce_edge : operation {
    int endpoint_a;
    int endpoint_b;

    //constructor
    operation_introduce_edge(int v, int w);
};

class nice_bag {
public:
    std::unique_ptr<operation> op; // allows for polymorphism.
    std::vector<int> bag;

    //overloading constructor
    nice_bag();
    nice_bag(operation_enum operation, std::vector<int>bag_input); //For leaf & join opperation.
    nice_bag(operation_enum operation, int v, std::vector<int>bag_input); //For Introduce and Forget opperation.
    nice_bag(operation_enum operation, int v, int w, std::vector<int>bag_input); //For Introduce_edge operation.
};

class TREE_DECOMPOSITION {
public:
	std::vector<std::vector<int>> bags;
    std::vector<nice_bag> nice_bags;
	adjacencyListBoost graph_td;
    adjacencyListBoost graph_nice_td;
	int treewidth;
    int root_vertex;

	// Constructor
	TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost g, int treewidth);

    int select_root_bag();

    void unfold_parent_vertex(int parent, int child);

    void create_nice_tree_decomposition();

    void traverse_tree_decomposition(int parent_index, vertex v);

    void unfold_leaf_vertex(int vertex);
};

std::vector<int> find_non_overlapping_vertices(const std::vector<int>& bag_parent, const std::vector<int>& bag_child);