//
// Created by floris on 3/17/25.
//
#pragma once

#include <variant>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <queue>
#include <htd/ITreeDecomposition.hpp>

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (container used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (container used for vertex List (vector))
    boost::undirectedS,                           // Param: undirected.
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indices
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indices
    adjacencyListBoost;

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (container used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (container used for vertex List (vector))
    boost::directedS,                           // Param: undirected.
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indices
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indices
    adjacencyListBoostDirected;

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

typedef boost::graph_traits<adjacencyListBoost>::edge_iterator edge_itt;

enum operation_enum {
    FORGET,
    JOIN,
    INTRODUCE,
    INTRODUCE_EDGE,
    LEAF
};

struct operation_join
{
    //constructor
    operation_join();
};

struct operation_leaf
{
    //constructor
    operation_leaf();
};

struct operation_forget
{
    //The vertex which get forgotten.
    int vertex;

    //constructor
    explicit operation_forget(int v);
};

struct operation_introduce
{
    //The vertex which get introduced.
    int vertex;

    explicit operation_introduce(int v);
};

struct operation_introduce_edge
{
    //The endpoints of the introduced edge.
    int source;
    int target;

    operation_introduce_edge(int source, int target);
};

class nice_bag {
public:
    std::variant<operation_join, operation_leaf, operation_introduce, operation_forget, operation_introduce_edge> op;
    std::vector<uint> bag;

    //overloading constructor
    nice_bag();
    nice_bag(operation_enum operation, std::vector<uint>bag_input); // Constructor for leaf & join operation.
    nice_bag(operation_enum operation, int v, std::vector<uint>bag_input); // Constructor for Introduce and Forget operation.
    nice_bag(operation_enum operation, int v, int w, std::vector<uint>bag_input); // Constructor for Introducing an edge.
};

class NICE_TREE_DECOMPOSITION
{
public:
    std::vector<std::vector<int>> bags; //not needed anymore.
    std::vector<nice_bag> nice_bags; //Nice tree decomposition bags.
    adjacencyListBoostDirected graph_nice_tree_decomposition;
    adjacencyListBoost graph_tree_decomposition; //Not needed anymore

    int root_vertex;

    explicit NICE_TREE_DECOMPOSITION(adjacencyListBoost& reduced_graph, const htd::ITreeDecomposition* decomposition);

    NICE_TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost g);

    int introduce_edge_smart(int source, int target, int parent, int child, adjacencyListBoost& original_graph);

    void introduce_all_edges_smart(adjacencyListBoost& original_graph);

    void introduce_all_edges(adjacencyListBoost& original_graph);

    void traverse_tree_decomposition(int parent_index, vertex v);

    // int select_root_bag();
    //
    // void unfold_parent_vertex(int parent, int child, const std::vector<int>& bag_parent, const std::vector<int>& bag_child);
    //
    // void unfold_leaf_vertex(int vertex);
};
