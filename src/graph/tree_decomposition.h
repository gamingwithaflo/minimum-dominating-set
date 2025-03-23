// #pragma once
//
// #include <vector>
//
// #include <queue>
//
// #include<stack>
//
// #include <variant>
// #include <boost/graph/adjacency_list.hpp>
// #include <cinttypes>
// #include<unordered_map>
// #include "nice_tree_decomposition.h"
// #include <boost/functional/hash.hpp>
//
// typedef boost::adjacency_list<
//     boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
//     boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
//     boost::undirectedS,                           // Param:Directed
//     boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
//     boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
//     adjacencyListBoost;
//
// typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;
//
// typedef boost::graph_traits<adjacencyListBoost>::edge_iterator edge_itt;
//
// std::vector<std::uint64_t> generate_all_encoding(int n);
//
// void generate_encoding(int n, int coloring, int position, std::vector<std::uint64_t>& results);
//
// std::vector<std::pair<std::uint64_t, std::uint64_t>> generate_all_encoding_introduce(int n, int index_introduced);
//
// void generate_encoding_introduce(int n, std::uint64_t coloring, std::uint64_t child_coloring, int position, int index_introduced, std::vector<std::pair<std::uint64_t, std::uint64_t>>& results);
//
// struct solution_struct {
//     int ref_count;
//     std::vector<int> solution;
//
//     solution_struct(std::vector<int> sol);
// };
//
// class TREE_DECOMPOSITION {
// public:
//     std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition_ptr;
//     std::stack<nice_bag*> instruction_stack;
//     std::stack<std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>>partial_solution_stack;
//     std::unordered_map<std::vector<int>, solution_struct, boost::hash<std::vector<int>>>local_solution;
//     std::vector<int> global_solution;
//
// 	// Constructor
// 	explicit TREE_DECOMPOSITION(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition);
//
//     void insert_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>& partial_solution, std::uint64_t encoding, std::vector<int> key, int size);
//
//     void remove_all_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int, solution_struct*>>& child_partial_solution_vector);
//
//     void fill_instruction_stack();
//
//     void depth_first_search(int start, int parent);
//
//     void run_instruction_stack(std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);
//
//     void run_operation_leaf();
//
//     void run_operation_introduce(std::vector<uint>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);
//
//     void run_operation_forget(std::vector<uint>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);
//
//     void run_operation_introduce_edge(std::vector<uint>& bag, int endpoint_a, int endpoint_b);
//
//     void run_operation_join(std::vector<uint>& bag);
//
//     void solve_root_vertex();
// };
//
// std::vector<int> find_non_overlapping_vertices(const std::vector<int>& bag_parent, const std::vector<int>& bag_child);