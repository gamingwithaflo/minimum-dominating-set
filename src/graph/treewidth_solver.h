#pragma once

#include <vector>
#include <stack>
#include "nice_tree_decomposition.h"


struct solution_struct {
    std::vector<int> solution;
    int ref_cnt;

    explicit solution_struct(const std::vector<int>& solution);
};

struct partial_solution {
    std::uint64_t encoding;
    int domination_number;
    solution_struct* solution;

    explicit partial_solution(std::uint64_t encoding, solution_struct* solution, int domination_number);
};

class TREEWIDTH_SOLVER {
public:
    std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition_ptr;
    std::stack<nice_bag*> instruction_stack;
    std::stack<std::vector<partial_solution>> partial_solution_stack;
    boost::unordered_map<std::vector<int>, solution_struct> local_solution;

    explicit TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void insert_entry_new_partial_solution(std::vector<partial_solution>& new_partial_solution, std::uint64_t encoding, std::vector<int>& solution, int domination_number);

    void remove_all_entries_partial_solution(std::vector<partial_solution>& child_partial_solutions);

    void fill_instruction_stack();

    void run_instruction_stack(std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void depth_first_search(int start);

    void run_operation_leaf();

    void run_operation_introduce(std::vector<int>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);

    void run_operation_forget(std::vector<int>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex);

    void run_operation_introduce_edge(std::vector<int>& bag, int endpoint_a, int endpoint_b);

    void run_operation_join(std::vector<int>& bag);

    void solve_root_vertex();
};

//helper functions.
int find_index_in_bag(std::vector<int>& bag, int element);

std::uint64_t remove_color_at_index(std::uint64_t encoding,int index);

std::uint64_t add_color_at_index(std::uint64_t encoding, int index);

std::uint64_t manipulate_color(int index_vertex, int bag_size);
