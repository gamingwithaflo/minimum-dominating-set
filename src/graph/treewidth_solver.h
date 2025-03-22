#pragma once

#include <vector>
#include <stack>
#include "nice_tree_decomposition.h"




struct partial_solution {
    std::uint64_t encoding;
    std::shared_ptr<std::vector<int>> solution;

    explicit partial_solution(std::uint64_t encoding, std::vector<int>& solution);
};

class TREEWIDTH_SOLVER {
public:
    std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition_ptr;
    std::stack<nice_bag*> instruction_stack;
    std::stack<std::vector<partial_solution>> partial_solution_stack;
    boost::unordered_map<std::vector<int>, std::shared_ptr<std::vector<int>>> local_solution;

    explicit TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void insert_entry_partial_solution();

    void remove_all_entries_partial_solution();

    void fill_instruction_stack();

    void run_instruction_stack(std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void depth_first_search(int start);

    void run_operation_leaf();

    void run_operation_introduce(std::vector<uint>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);

    void run_operation_forget(std::vector<uint>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);

    void run_operation_introduce_edge(std::vector<uint>& bag, int endpoint_a, int endpoint_b);

    void run_operation_join(std::vector<uint>& bag);

    void solve_root_vertex();
};

//helper functions.
int find_index_in_bag(std::vector<int>& bag, int element);

std::uint64_t remove_color_at_index(std::uint64_t encoding,int index);

std::uint64_t add_color_at_index(std::uint64_t encoding, int index);

std::uint64_t manipulate_color(int index_vertex, int bag_size);
