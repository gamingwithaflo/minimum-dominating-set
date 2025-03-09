#pragma once

#include <vector>

#include <queue>

#include<stack>

#include <variant>
#include <boost/graph/adjacency_list.hpp>
#include <cinttypes>
#include<unordered_map>

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

typedef boost::graph_traits<adjacencyListBoost>::edge_iterator edge_itt;

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

struct operation_join : public operation {

    //constructor
    operation_join();
};

struct operation_leaf : public operation {

    //constructor
    operation_leaf();
};

struct operation_forget : public operation {
    int vertex;

    //constructor
    operation_forget(int v);
};


struct operation_introduce : public operation {
    int vertex;

    //constructor
    operation_introduce(int v);
};

struct operation_introduce_edge : public operation {
    int endpoint_a;
    int endpoint_b;

    //constructor
    operation_introduce_edge(int v, int w);
};

class nice_bag {
public:
    std::variant<operation_join, operation_leaf, operation_introduce, operation_forget, operation_introduce_edge> op;
    std::vector<int> bag;

    //overloading constructor
    nice_bag();
    nice_bag(operation_enum operation, std::vector<int>bag_input); //For leaf & join opperation.
    nice_bag(operation_enum operation, int v, std::vector<int>bag_input); //For Introduce and Forget opperation.
    nice_bag(operation_enum operation, int v, int w, std::vector<int>bag_input); //For Introduce_edge operation.
};

std::vector<std::pair<std::uint64_t, std::uint64_t>> generate_all_encoding_introduce(int n, int index_introduced);

std::vector<std::pair<std::uint64_t, std::uint64_t>>generate_encoding_introduce(int n, std::uint64_t coloring, std::uint64_t child_coloring, int position, int index_introduced, std::vector<std::pair<std::uint64_t, std::uint64_t>>& results);

class TREE_DECOMPOSITION {
public:
	std::vector<std::vector<int>> bags;
    std::vector<nice_bag> nice_bags;
	adjacencyListBoost graph_td;
    adjacencyListBoost graph_nice_td;
    std::stack<nice_bag*> instruction_stack;
    std::stack<std::unordered_map<std::uint64_t, int>>partial_solution_stack;

	int treewidth;
    int root_vertex;

	// Constructor
	TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost g, int treewidth);

    int select_root_bag();

    void introduce_all_edges(std::pair< edge_itt, edge_itt> edges_itterator);

    void unfold_parent_vertex(int parent, std::vector<int>& bag_parent, int child, std::vector<int>& bag_child);

    void create_nice_tree_decomposition(std::pair< edge_itt, edge_itt> edges_itterator);

    void traverse_tree_decomposition(int parent_index, vertex v);

    void unfold_leaf_vertex(int vertex);

    void fill_instruction_stack();

    void depth_first_search(int start, int parent);

    void run_instruction_stack();

    void run_operation_leaf();

    void run_operation_introduce(std::vector<int>& bag, int introduced_vertex);

    void run_operation_forget();

    void run_operation_introduce_edge();

    void run_operation_join();
};

std::vector<int> find_non_overlapping_vertices(const std::vector<int>& bag_parent, const std::vector<int>& bag_child);