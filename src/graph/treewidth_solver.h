#pragma once

#include <vector>
#include <stack>
#include "nice_tree_decomposition.h"
#include <memory>
#include <unordered_map>
#include <cinttypes>



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
    std::vector<int> global_solution;

    explicit TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void insert_entry_new_partial_solution(std::vector<partial_solution>& new_partial_solution, std::uint64_t encoding, std::vector<int>& solution, int domination_number);

    void remove_all_entries_partial_solution(std::vector<partial_solution>& child_partial_solutions);

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
int find_index_in_bag(const std::vector<uint>& bag, const int element) ;

std::uint64_t remove_color_at_index(std::uint64_t encoding,int index, int bag_size);

std::uint64_t add_color_at_index(std::uint64_t encoding, int index, int bag_size);

std::uint64_t manipulate_color_default(int index_vertex, int bag_size);

int extract_bits(std::uint64_t encoding, int size_bag, int pos);

int count_white_vertices(std::uint64_t encoding);

std::uint64_t create_compliment_encoding(std::uint64_t encoding);

std::uint64_t create_parent_join(std::uint64_t encoding);

bool contains_no_gray(std::uint64_t encoding);

std::vector<int> get_white_indices(std::uint64_t encoding, int num_of_pairs);

std::vector<int> get_gray_indices(std::uint64_t encoding, int num_of_pairs);

void generate_combination(partial_solution& child,
                          boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                          boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                          std::vector<std::uint64_t>& parent_encodings,
                          int number_of_gray_fixed,
                          int bag_size,
                          std::vector<int>& gray_indices);

void update_best_combinations_join(partial_solution& child,
                                                     boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                                                     boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                                                     std::vector<std::uint64_t>& parent_encodings,
                                                     std::uint64_t compliment_encoding,
                                                     std::uint64_t parent_encoding);

std::uint64_t create_parent_encoding_ignore(std::uint64_t encoding, int bag_size, std::vector<int>& index_ignored_vertices);

std::uint64_t create_compliment_encoding_ignore(std::uint64_t encoding, int bag_size, std::vector<int>& index_ignored_vertices);
