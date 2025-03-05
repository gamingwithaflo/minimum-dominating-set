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

class TREE_DECOMPOSITION {
public:
	std::vector<std::vector<int>> bags;
	adjacencyListBoost graph;
	int treewidth;

	// Constructor
	TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost& g, int treewidth);

    void create_nice_tree_decomposition();
};

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
    operation opp;
    std::vector<int> bag;

};