#pragma once

#include "treewidth_solver.h"
#include "../util/timer.h"
#include <iterator>
#include <algorithm>

#include <iostream>

TREEWIDTH_SOLVER::TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<bool>& dominated, std::vector<bool>&excluded, std::unordered_map<int,int>& newToOldIndex) {
    //initialize
    this->nice_tree_decomposition_ptr = std::move(nice_tree_decomposition);
    instruction_stack;
    partial_solution_stack;
    local_solution;
    global_solution;

    //run.
    fill_instruction_stack();
    run_instruction_stack(dominated, excluded, newToOldIndex);
}

void TREEWIDTH_SOLVER::insert_entry_new_partial_solution(std::vector<partial_solution>& new_partial_solution, std::uint64_t encoding, boost::dynamic_bitset<>& solution, int domination_number) {
    solution_struct* ptr_solution = nullptr;

    auto it = local_solution.find(solution);

    if (it == local_solution.end()) {
        //Using existing vector
        auto [new_it, succes] = local_solution.emplace(solution, solution_struct(solution));
        ptr_solution = &new_it->second;
    } else {
        ptr_solution = &(it->second);
        it->second.ref_cnt++;
    }

    new_partial_solution.emplace_back(encoding, ptr_solution, domination_number);
}

void TREEWIDTH_SOLVER::remove_all_entries_partial_solution(std::vector<partial_solution>& child_partial_solutions) {
    for (auto& child_partial_solution : child_partial_solutions) {
        child_partial_solution.solution->ref_cnt--;

        //garbage collection.
        if (child_partial_solution.solution->ref_cnt == 0) {
            local_solution.erase(child_partial_solution.solution->solution);
        }
    }
}

void TREEWIDTH_SOLVER::fill_instruction_stack() {
    depth_first_search(nice_tree_decomposition_ptr->root_vertex);
}

//depth first traversal. it splits on join vertices.
void TREEWIDTH_SOLVER::depth_first_search(int start) {
    instruction_stack.push(&nice_tree_decomposition_ptr->nice_bags[start]);
    auto [itt, itt_end] = boost::out_edges(start, nice_tree_decomposition_ptr->graph_nice_tree_decomposition);
    for (; itt != itt_end; ++itt) {
        const int depth = boost::target(*itt, nice_tree_decomposition_ptr->graph_nice_tree_decomposition);
        depth_first_search(depth);
    }
}

void TREEWIDTH_SOLVER::run_instruction_stack(std::vector<bool>& dominated, std::vector<bool>& excluded, std::unordered_map<int, int>& newToOldIndex) {
    while (!instruction_stack.empty()) {
        //get top instruction from the stack. (a pointer).
        nice_bag* instruction_ptr = instruction_stack.top();
        instruction_stack.pop();
        nice_bag& instruction = *instruction_ptr;

        //find which operation the instruction entails.

        //run if it is a leaf operation.
        if (std::holds_alternative<operation_leaf>(instruction.op)) {
            run_operation_leaf(newToOldIndex.size());
            continue;
        }

        if (std::holds_alternative<operation_introduce>(instruction.op)) {
            //get object.
            operation_introduce correct_instruction = get<operation_introduce>(instruction.op);
            run_operation_introduce(instruction.bag, correct_instruction.vertex, dominated, excluded, newToOldIndex);
            continue;
        }

        if (std::holds_alternative<operation_forget>(instruction.op)) {
            //get object.
            operation_forget correct_instruction = get<operation_forget>(instruction.op);
            run_operation_forget(instruction.bag, correct_instruction.vertex, excluded, newToOldIndex);
            continue;
        }

        if (std::holds_alternative<operation_introduce_edge>(instruction.op)) {
            //get object.
            operation_introduce_edge correct_instruction = get<operation_introduce_edge>(instruction.op);
            run_operation_introduce_edge(instruction.bag, correct_instruction.source, correct_instruction.target);
            continue;
        }

        if (std::holds_alternative<operation_join>(instruction.op)) {
            run_operation_join(instruction.bag);
            continue;
        }
    }
    solve_root_vertex();
}

//(2-bit representation of colors of vertices). 0b01 -> black, 0b11 -> gray, 0b10 -> white.
const int COLORS[] = { 0b01, 0b11, 0b10 };
const int BLACK = 1;
const int WHITE = 2;
const int GRAY = 3;
const int NUM_COLORS = 3;

void TREEWIDTH_SOLVER::run_operation_leaf(int num_of_vertices) {
    timer t_operation_leaf;
    std::vector<partial_solution> partial_solutions;
    boost::dynamic_bitset<> empty_solution(num_of_vertices);
    insert_entry_new_partial_solution(partial_solutions, 0, empty_solution, 0);
    partial_solution_stack.push(partial_solutions);
    //std::cout << "run operation leaf" << std::endl;
    Logger::execution_time_leaf += t_operation_leaf.count();
}

void TREEWIDTH_SOLVER::run_operation_introduce(std::vector<uint>& bag, int introduced_vertex, std::vector<bool>& dominated, std::vector<bool>& excluded, std::unordered_map<int, int>& newToOldIndex){
    timer t_operation_introduce;
    int index_introduced_vertex = find_index_in_bag(bag, introduced_vertex);

    //get previous partial solution.
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();
    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(child_partial_solution.size() * 3); // in the worst case.

    for (auto& child_encoding : child_partial_solution){
        //generate possible parent encodings.
        std::uint64_t parent_encoding = add_color_at_index(child_encoding.encoding, index_introduced_vertex, bag.size());
        std::uint64_t increase_color = manipulate_color_default(index_introduced_vertex, bag.size());

        //handle black vertices.
        std::uint64_t parent_encoding_black = parent_encoding + increase_color; // black encoding : 01
        if (dominated[newToOldIndex[introduced_vertex]]){
            //Child bag was a leaf.
            const int domination_number_black = child_encoding.domination_number; // plus 0.
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_black, child_encoding.solution->solution, domination_number_black);
        }
        //handle white vertices.
        std::uint64_t parent_encoding_white = parent_encoding + (2 * increase_color); // White encoding : 10
        const int domination_number_white = child_encoding.domination_number + 1;
        insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_white, child_encoding.solution->solution, domination_number_white);

        std::uint64_t parent_encoding_gray = parent_encoding + (3 * increase_color); // gray encoding : 11
        const int domination_number_gray = child_encoding.domination_number; // plus 0.
        insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_gray, child_encoding.solution->solution, domination_number_gray);
    }
    remove_all_entries_partial_solution(child_partial_solution);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);

    Logger::execution_time_introduce += t_operation_introduce.count();
}

void TREEWIDTH_SOLVER::run_operation_join(std::vector<uint>& bag){
    timer t_operation_join;
    std::vector<partial_solution> temp_child_partial_solution_a = partial_solution_stack.top();
    partial_solution_stack.pop();
    std::vector<partial_solution>& child_partial_solution_b = partial_solution_stack.top();

    auto& smaller_map = (temp_child_partial_solution_a.size() <= child_partial_solution_b.size())
                           ? temp_child_partial_solution_a
                           : child_partial_solution_b;
    auto& bigger_map = (temp_child_partial_solution_a.size() > child_partial_solution_b.size())
                            ? temp_child_partial_solution_a
                            : child_partial_solution_b;

    boost::unordered_map<std::uint64_t, partial_solution> child_partial_solution_a;
    for (auto& child : bigger_map) {
        child_partial_solution_a.insert({child.encoding, child});
    }


    boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>> best_combinations;

    std::vector<std::uint64_t> parent_encodings;
    parent_encodings.reserve(3 * smaller_map.size());


    for (auto& child : smaller_map){
        std::uint64_t find_gray = create_find_gray(child.encoding);
        std::uint64_t find_black = create_find_black(child.encoding);
        const std::uint64_t compliment_encoding = create_compliment_encoding(child.encoding, find_gray, find_black);
        const std::uint64_t parent_encoding = create_parent_encoding(child.encoding, find_gray);

        std::vector<int> gray_indices = get_gray_indices(child.encoding, bag.size());

        if (gray_indices.empty()){
            //No gray vertices.
            update_best_combinations_join(child, best_combinations, child_partial_solution_a, parent_encodings, compliment_encoding, parent_encoding);
        } else {
            //combinations if i == 0; (no GRAY vertex is fixed).
            update_best_combinations_join(child, best_combinations, child_partial_solution_a, parent_encodings, compliment_encoding, parent_encoding);

            for (int i = 1; i <= gray_indices.size(); ++i){
                generate_combination(child, best_combinations, child_partial_solution_a, parent_encodings, i, bag.size(), gray_indices, find_black);
            }
        }
    }
    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(parent_encodings.size());
    //boost::unordered_map<std::pair<std::vector<int>, std::vector<int>>, std::vector<int>> batch_merge;
    for (auto& parent_encoding : parent_encodings){
        auto [partial_solution_a, partial_solution_b] = best_combinations[parent_encoding];
        int domination_number = partial_solution_a->domination_number + partial_solution_b->domination_number - count_white_vertices(parent_encoding);
        //std::vector<int> solution = {};
        auto partial_solution_a_bitset = partial_solution_a->solution->solution;
        auto partial_solution_b_bitset = partial_solution_b->solution->solution;
        boost::dynamic_bitset<> solution = partial_solution_a_bitset | partial_solution_b_bitset;
        insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, solution, domination_number);
        //}
    }
    remove_all_entries_partial_solution(child_partial_solution_b);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);
    remove_all_entries_partial_solution(temp_child_partial_solution_a);
    Logger::execution_time_join += t_operation_join.count();
}

void TREEWIDTH_SOLVER::run_operation_introduce_edge(std::vector<uint>& bag, int endpoint_a, int endpoint_b){
    timer t_operation_introduce_edge;
    //find index of introduced vertex in the bag.
    int index_endpoint_a = find_index_in_bag(bag, endpoint_a);
    int index_endpoint_b = find_index_in_bag(bag, endpoint_b);

    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(2 * child_partial_solution.size());

    for (auto& child_encoding : child_partial_solution){
        int color_endpoint_a = extract_bits(child_encoding.encoding, bag.size(), index_endpoint_a);
        int color_endpoint_b = extract_bits(child_encoding.encoding, bag.size(), index_endpoint_b);

        if (color_endpoint_a == BLACK && color_endpoint_b == WHITE){
            continue;
        }
        if (color_endpoint_a == WHITE && color_endpoint_b == BLACK){
            continue;
        }
        if (color_endpoint_a == WHITE && color_endpoint_b == GRAY){
            std::uint64_t manipulate_color_b = manipulate_color_default(index_endpoint_b, bag.size());
            std::uint64_t parent_encoding = child_encoding.encoding - (2 * manipulate_color_b);
            // first copy entry.
            insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, child_encoding.solution->solution, child_encoding.domination_number);
            continue;
        }
        if (color_endpoint_a == GRAY && color_endpoint_b == WHITE)
        {
            std::uint64_t manipulate_color_a = manipulate_color_default(index_endpoint_a, bag.size());
            std::uint64_t parent_encoding = child_encoding.encoding - (2 * manipulate_color_a);
            // first copy entry.
            insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, child_encoding.solution->solution, child_encoding.domination_number);
            continue;
        }
        insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
    }
    remove_all_entries_partial_solution(child_partial_solution);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);

    Logger::execution_time_introduce_edge += t_operation_introduce_edge.count();
}

void TREEWIDTH_SOLVER::run_operation_forget(std::vector<uint>& bag, int forget_vertex, std::vector<bool>& excluded, std::unordered_map<int, int>& newToOldIndex){
    timer t_operation_forget;
    int index_forget_vertex = find_index_in_bag(bag, forget_vertex);

    //get previous partial solution.
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    std::vector<std::uint64_t> parent_encodings;
    parent_encodings.reserve(child_partial_solution.size());
    boost::unordered_map<std::uint64_t,partial_solution*> temp_partial_solutions; //parent encoding, child.

    for (auto& child_encoding : child_partial_solution){
        std::uint64_t parent_encoding = remove_color_at_index(child_encoding.encoding, index_forget_vertex, bag.size());

        const int color = extract_bits(child_encoding.encoding, bag.size()+1, index_forget_vertex);
        if (color == GRAY) {
            continue;
        }
        if (color == BLACK) {
            if (auto it = temp_partial_solutions.find(parent_encoding); it == temp_partial_solutions.end()){
                temp_partial_solutions.insert({parent_encoding, &child_encoding});
                parent_encodings.push_back(parent_encoding);
            } else {
                if (child_encoding.domination_number < it->second->domination_number){
                    temp_partial_solutions[parent_encoding] = &child_encoding;
                }
            }
            continue;
        }
        if (color == WHITE) {
            if (excluded[newToOldIndex[forget_vertex]] == 1){
                continue;
            }
            if (auto it = temp_partial_solutions.find(parent_encoding); it == temp_partial_solutions.end()){
                temp_partial_solutions.insert({parent_encoding, &child_encoding});
                parent_encodings.push_back(parent_encoding);
            } else {
                if (child_encoding.domination_number < it->second->domination_number){
                    temp_partial_solutions[parent_encoding] = &child_encoding;
                }
            }
        }
        else
        {
            throw std::runtime_error("this should not happen");
        }
    }
    std::vector<partial_solution> new_partial_solution;
    new_partial_solution.reserve(parent_encodings.size());

    for (auto& parent_encoding : parent_encodings){
        const auto best_child = temp_partial_solutions[parent_encoding];
        const int color = extract_bits(best_child->encoding, bag.size()+1, index_forget_vertex);
        if (color == BLACK){
            //no new introduces.
            insert_entry_new_partial_solution(new_partial_solution, parent_encoding, best_child->solution->solution, best_child->domination_number);
            continue;
        }
        if (color == WHITE) {
            auto solution = best_child->solution->solution;
            solution.flip(forget_vertex);
            insert_entry_new_partial_solution(new_partial_solution, parent_encoding, solution, best_child->domination_number);
            continue;
        }
        if  (color == GRAY) {
            continue;
        }
        throw std::runtime_error("should not be anything else");
    }
    remove_all_entries_partial_solution(child_partial_solution);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solution);
    Logger::execution_time_forget += t_operation_forget.count();
}

void TREEWIDTH_SOLVER::solve_root_vertex() {
    std::vector<uint>& bag_root_vertex = nice_tree_decomposition_ptr->nice_bags[nice_tree_decomposition_ptr->root_vertex].bag;
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    int lowest_domination_number = INT_MAX;
    partial_solution* lowest_encoding = nullptr;

    for (auto& child : child_partial_solution){
        if (contains_no_gray(child.encoding)){
            if (lowest_domination_number > child.domination_number){
                lowest_domination_number = child.domination_number;
                lowest_encoding = &child;
            }
        }
    }
    if (lowest_encoding == nullptr){
        throw std::runtime_error("should never be null");
    }
    auto solution = lowest_encoding->solution->solution;
    std::vector<int> white_indices = get_white_indices(lowest_encoding->encoding, bag_root_vertex.size());
    for (int index : white_indices) {
        solution.flip(bag_root_vertex[index]);
    }
    global_solution.reserve(lowest_domination_number);
    for (int i = 0; i < solution.size(); ++i) {
        if (solution[i] == 1)
        {
            global_solution.push_back(i);
        }
    }
    partial_solution_stack.pop();
}

solution_struct::solution_struct(const boost::dynamic_bitset<>& solution) {
    this->solution = solution;
    this->ref_cnt = 1;
}

partial_solution::partial_solution(std::uint64_t encoding, solution_struct* solution, int domination_number) {
    this->encoding = encoding;
    this->solution = solution;
    this->domination_number = domination_number;
}

bool contains_no_gray(std::uint64_t encoding) {
    // Create a mask that isolates all even-positioned bit-pairs
    std::uint64_t mask = 0x5555555555555555; // 0b01 repeated (ensures checking pairs)

    // Check if value has any '11' pairs
    return (encoding & (encoding >> 1) & mask) == 0;
}

//Helper function. Assumption: bag is ordered.
//either returns the index of the element, or the location where element should be inserted (to keep the bag sorted).
int find_index_in_bag(const std::vector<uint>& bag, const int element) {
    auto it = std::lower_bound(bag.begin(), bag.end(), element); // point to the first element equal or bigger than the element.

    return it - bag.begin(); //returns the index.
}

std::uint64_t manipulate_color_default(const int index_vertex, const int bag_size){
    int power = bag_size - 1 - index_vertex;
    return 1 << (2 * power); //equivalent to 4^power
}

//Helper function.
std::uint64_t add_color_at_index(std::uint64_t encoding, int index, int bag_size) {
    int correct_index = (bag_size - 1) - index;
    std::uint64_t mask_prev_color;
    if (correct_index == 0) {
        return encoding << 2;
    }
    mask_prev_color = (1 << correct_index * 2) | (1 << (correct_index * 2 + 1));
    const std::uint64_t mask = 0xFFFFFFFFFFFFFFFF << ((2 * correct_index)); // need 2 extra so (newly introduced color is empty (00)).
    return ((((encoding ^ (encoding << 2)) & mask) ^ encoding) & (~mask_prev_color));
}

//Helper function. removes 2 bits (a color) at the index of the bag.
std::uint64_t remove_color_at_index(const std::uint64_t encoding, const int index, int bag_size) {
    int new_index = bag_size - index;
    if (new_index == 0) {
        return encoding >> 2;
    }
    const std::uint64_t mask = 0xFFFFFFFFFFFFFFFF << (2 * new_index); // 0xF = 11.
    return ((encoding ^ (encoding >> 2)) & mask) ^ encoding;
}

//size_bag should be the size of the encoding.
int extract_bits(std::uint64_t encoding, int size_bag, int pos) {
    return (encoding >> (2 * (size_bag - 1 - pos))) & 0b11;
}

int count_white_vertices(std::uint64_t encoding) {
    uint64_t mask = 0x5555555555555555; // 0b01 repeated (ensures checking pairs)
    return __builtin_popcountll((~encoding) & (encoding >> 1) & mask); // ~ = bitflip, & = AND. & popcountll counts the number of 1 bits.
}

std::vector<int> get_white_indices(std::uint64_t encoding, int num_of_pairs) {
    std::vector<int> whiteIndices;

    // Start from the most significant 2-bit pair
    for (int i = num_of_pairs - 1; i >= 0; --i) {
        uint64_t pair = (encoding >> (i * 2)) & 0b11; // Extract 2-bit pair
        if (pair == 0b10) { // Check if it is '10' (white)
            whiteIndices.push_back((num_of_pairs - 1) - i); // Adjust index to match left-to-right order
        }
    }

    return whiteIndices;
}

std::vector<int> get_gray_indices(std::uint64_t encoding, int num_of_pairs){
    std::vector<int> grayIndices;

    //Start from the most significant 2-bit pair
    for (int i = num_of_pairs - 1; i >= 0; --i) {
        uint64_t pair = (encoding >> (i * 2)) & 0b11;
        if (pair == 0b11) {
            grayIndices.push_back((num_of_pairs - 1) - i);
        }
    }
    return grayIndices;
}

void update_best_combinations_join(partial_solution& child,
                                                     boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                                                     boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                                                     std::vector<std::uint64_t>& parent_encodings,
                                                     std::uint64_t compliment_encoding,
                                                     std::uint64_t parent_encoding){

    if (auto complement_encoding_a = child_partial_solution_a.find(compliment_encoding); complement_encoding_a != child_partial_solution_a.end())
    {
        if (auto i = best_combinations.find(parent_encoding); i == best_combinations.end()){
            //create first.
            parent_encodings.emplace_back(parent_encoding);
            best_combinations[parent_encoding] = {&child, &complement_encoding_a->second};
        } else
        {
            //check if this solution is better.
            int result_exist = i->second.first->domination_number + i->second.second->domination_number;
            int result_new = child.domination_number + complement_encoding_a->second.domination_number;
            if (result_exist > result_new){
                best_combinations[parent_encoding] = {&child, &complement_encoding_a->second};
            }
            //else do nothing.
        }
    }
}

//
void generate_combination(partial_solution& child,
                          boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>>& best_combinations,
                          boost::unordered_map<std::uint64_t, partial_solution>& child_partial_solution_a,
                          std::vector<std::uint64_t>& parent_encodings,
                          int number_of_gray_fixed,
                          int bag_size,
                          std::vector<int>& gray_indices,
                          std::uint64_t find_black) {
    std::string bitmask(number_of_gray_fixed, 1);
    bitmask.resize(gray_indices.size(), 0);

    std::vector<int> index_ignored_gray_indices;
    index_ignored_gray_indices.reserve(gray_indices.size());
    do {
        index_ignored_gray_indices.clear();
        std::uint64_t find_gray = create_find_gray(child.encoding);
        find_gray = find_gray >> 1;
        for (int i = 0; i < gray_indices.size(); ++i){
            if (bitmask[i]){
                const int new_index = bag_size - 1 - gray_indices[i];
                find_gray ^= (1 << (new_index * 2));
            }
        }
        find_gray = find_gray << 1;

        std::uint64_t parent_encoding = create_parent_encoding(child.encoding, find_gray);
        std::uint64_t compliment_encoding = create_compliment_encoding(child.encoding, find_gray, find_black);
        update_best_combinations_join(child, best_combinations, child_partial_solution_a, parent_encodings, compliment_encoding, parent_encoding);

    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

std::uint64_t create_parent_encoding(const std::uint64_t encoding, const std::uint64_t& find_gray){
    return find_gray ^ encoding;
}

//makes all GRAY encodings BLACK and all BLACK encodings GRAY.
std::uint64_t create_compliment_encoding(std::uint64_t encoding, std::uint64_t& find_gray, std::uint64_t& find_black) {
    return ((find_gray | find_black)) ^ encoding;
}

std::uint64_t create_find_black(std::uint64_t encoding) {
    const std::uint64_t mask = 0x5555555555555555;
    const std::uint64_t find_black = ((encoding & (~encoding >> 1) & mask));
    return find_black << 1;
}

std::uint64_t create_find_gray(std::uint64_t encoding) {
    const std::uint64_t mask = 0x5555555555555555;
    const std::uint64_t find_gray = ((encoding & (encoding >> 1) & mask));
    return find_gray << 1;
}
