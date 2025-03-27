#pragma once

#include <vector>
#include <stack>
#include "nice_tree_decomposition.h"
#include <memory>
#include <unordered_map>
#include <cinttypes>
#include <boost/dynamic_bitset.hpp>
#include "../util/logger.h"



struct solution_struct {
    int ref_cnt;
    boost::dynamic_bitset<> solution;
    boost::dynamic_bitset<> dominated;

    solution_struct(const boost::dynamic_bitset<>& solution, const boost::dynamic_bitset<>& dominated);
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
    boost::unordered_map<boost::dynamic_bitset<>, solution_struct> local_solution;
    std::vector<int> global_solution;
    std::vector<boost::dynamic_bitset<>>dominated_vector;

    explicit TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex, std::vector<boost::dynamic_bitset<>>& dominated_vector);

    void insert_entry_new_partial_solution(std::vector<partial_solution>& new_partial_solution, std::uint64_t encoding, boost::dynamic_bitset<>& solution,boost::dynamic_bitset<>& dominated, int domination_number);

    void remove_all_entries_partial_solution(std::vector<partial_solution>& child_partial_solutions);

    void fill_instruction_stack();

    void run_instruction_stack(std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex);

    void depth_first_search(int start);

    void run_operation_leaf(int num_of_vertices, boost::dynamic_bitset<>& dominated_solution);

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

bool contains_no_gray(std::uint64_t encoding);

std::vector<int> get_white_indices(std::uint64_t encoding, int num_of_pairs);

std::vector<int> get_gray_indices(std::uint64_t encoding, int num_of_pairs);

void generate_combination(partial_solution& child,
                          boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                          boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                          std::vector<std::uint64_t>& parent_encodings,
                          int number_of_gray_fixed,
                          int bag_size,
                          std::vector<int>& gray_indices,
                          std::uint64_t find_black);

void update_best_combinations_join(partial_solution& child,
                                                     boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                                                     boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                                                     std::vector<std::uint64_t>& parent_encodings,
                                                     std::uint64_t compliment_encoding,
                                                     std::uint64_t parent_encoding);

std::uint64_t create_parent_encoding(std::uint64_t encoding, const std::uint64_t& find_gray);

std::uint64_t create_compliment_encoding(std::uint64_t encoding, std::uint64_t& find_gray, std::uint64_t& find_black);

std::uint64_t create_find_black(std::uint64_t encoding);

std::uint64_t create_find_gray(std::uint64_t encoding);

