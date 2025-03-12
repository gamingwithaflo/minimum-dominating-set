#pragma once

#include <vector>

#include <queue>

#include<stack>

#include <variant>
#include <boost/graph/adjacency_list.hpp>
#include <cinttypes>
#include<unordered_map>

#include <boost/functional/hash.hpp>

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

std::vector<std::uint64_t> generate_all_encoding(int n);

void generate_encoding(int n, int coloring, int position, std::vector<std::uint64_t>& results);

std::vector<std::pair<std::uint64_t, std::uint64_t>> generate_all_encoding_introduce(int n, int index_introduced);

void generate_encoding_introduce(int n, std::uint64_t coloring, std::uint64_t child_coloring, int position, int index_introduced, std::vector<std::pair<std::uint64_t, std::uint64_t>>& results);

struct solution_struct {
    int ref_count;
    std::vector<int> solution;

    solution_struct(std::vector<int> sol);
};

class TREE_DECOMPOSITION {
public:
	std::vector<std::vector<int>> bags;
    std::vector<nice_bag> nice_bags;
	adjacencyListBoost graph_td;
    adjacencyListBoost graph_nice_td;
    std::stack<nice_bag*> instruction_stack;
    std::stack<std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>>partial_solution_stack;
    std::unordered_map<std::vector<int>, solution_struct, boost::hash<std::vector<int>>>local_solution;
    std::vector<int> global_solution;

	int treewidth;
    int root_vertex;

	// Constructor
	TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost g, int treewidth);

    void introduce_all_edges_default(adjacencyListBoost& original_graph);

    int select_root_bag();

    void insert_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>& partial_solution, std::uint64_t encoding, std::vector<int> key, int size);

    void remove_all_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>& child_partial_solution_vector);

    void introduce_all_edges(std::pair< edge_itt, edge_itt> edges_itterator);

    void unfold_parent_vertex(int parent, std::vector<int>& bag_parent, int child, std::vector<int>& bag_child);

    void create_nice_tree_decomposition(adjacencyListBoost& original_graph);

    void traverse_tree_decomposition(int parent_index, vertex v);

    void unfold_leaf_vertex(int vertex);

    void fill_instruction_stack();

    void depth_first_search(int start, int parent);

    void run_instruction_stack();

    void run_operation_leaf();

    void run_operation_introduce(std::vector<int>& bag, int introduced_vertex);

    void run_operation_forget(std::vector<int>& bag, int forget_vertex);

    void run_operation_introduce_edge(std::vector<int>& bag, int endpoint_a, int endpoint_b);

    void run_operation_join(std::vector<int>& bag);

    void solve_root_vertex();
};

std::vector<int> find_non_overlapping_vertices(const std::vector<int>& bag_parent, const std::vector<int>& bag_child);