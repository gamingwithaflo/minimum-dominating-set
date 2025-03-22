#include "treewidth_solver.h"

TREEWIDTH_SOLVER::TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex) {
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

void TREEWIDTH_SOLVER::insert_entry_new_partial_solution(std::vector<partial_solution>& new_partial_solution, std::uint64_t encoding, std::vector<int>& solution, int domination_number) {
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

void TREEWIDTH_SOLVER::run_instruction_stack(std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex) {
    while (!instruction_stack.empty()) {
        //get top instruction from the stack. (a pointer).
        nice_bag* instruction_ptr = instruction_stack.top();
        instruction_stack.pop();
        nice_bag& instruction = *instruction_ptr;

        //find which operation the instruction entails.

        //run if it is a leaf operation.
        if (std::holds_alternative<operation_leaf>(instruction.op)) {
            run_operation_leaf();
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

void TREEWIDTH_SOLVER::run_operation_leaf(){
    std::vector<partial_solution> partial_solutions;
    partial_solution_stack.push(partial_solutions);
}

void TREEWIDTH_SOLVER::run_operation_introduce(std::vector<uint>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex){
    int index_introduced_vertex = find_index_in_bag(bag, introduced_vertex);

    //get previous partial solution.
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();
    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(child_partial_solution.size() * 3); // in the worst case.

    for (auto& child_encoding : child_partial_solution){
        //generate possible parent encodings.
        std::uint64_t parent_encoding = add_color_at_index(child_encoding.encoding, index_introduced_vertex);
        std::uint64_t increase_color = manipulate_color(index_introduced_vertex, bag.size());

        //handle black vertices.
        std::uint64_t parent_encoding_black = parent_encoding + increase_color; // black encoding : 01
        if (dominated[newToOldIndex[introduced_vertex]]){
            //Child bag was a leaf.
            if (child_partial_solution.empty()){
                std::vector<int> empty;
                insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_black, empty, 0);
            } else {
                const int domination_number = child_encoding.domination_number; // plus 0.
                insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_black, child_encoding.solution->solution, domination_number);
            }
        }
        //handle white vertices.
        std::uint64_t parent_encoding_white = parent_encoding + (2 * increase_color); // black encoding : 10
        if (child_partial_solution.empty()) {
            std::vector<int> empty;
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_white, empty, 1);
        } else {
            const int domination_number = child_encoding.domination_number + 1;
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_white, child_encoding.solution->solution, domination_number);
        }

        std::uint64_t parent_encoding_gray = parent_encoding + (3 * increase_color); // gray encoding : 11
        if (child_partial_solution.empty()) {
            std::vector<int> empty;
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_gray, empty, 0);
        } else {
            const int domination_number = child_encoding.domination_number; // plus 0.
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding_gray, child_encoding.solution->solution, domination_number);
        }
    }
    remove_all_entries_partial_solution(new_partial_solutions);
    partial_solution_stack.pop();
    partial_solution_stack.push(child_partial_solution);
}

void TREEWIDTH_SOLVER::run_operation_join(std::vector<uint>& bag){
    std::vector<partial_solution> temp_child_partial_solution_a = partial_solution_stack.top();
    boost::unordered_map<std::uint64_t, partial_solution> child_partial_solution_a;
    for (auto& child : temp_child_partial_solution_a){
        child_partial_solution_a.insert({child.encoding, child});
    }
    partial_solution_stack.pop();

    std::vector<partial_solution>& child_partial_solution_b = partial_solution_stack.top();

    boost::unordered_map<std::uint64_t, std::pair<partial_solution*, partial_solution*>> best_combinations;

    std::vector<std::uint64_t> parent_encodings;
    parent_encodings.reserve(child_partial_solution_b.size());

    for (auto& child : child_partial_solution_b){
        std::uint64_t compliment_encoding = create_compliment_encoding(child.encoding);
        std::uint64_t parent_encoding = create_parent_join(compliment_encoding);
        if (auto it = child_partial_solution_a.find(compliment_encoding); it == child_partial_solution_a.end()){
            continue;
        } else {
            if (auto itt = best_combinations.find(parent_encoding); itt == best_combinations.end()){
                //create first.
                best_combinations[parent_encoding] = {&child, &it->second};
            } else {
                //check if this solution is better.
                int result_exist = itt->second.first->domination_number + itt->second.second->domination_number;
                int result_new = child.domination_number + it->second.domination_number;
                if (result_exist > result_new){
                    best_combinations[parent_encoding] = {&child, &it->second};
                }
                //else do nothing.
            }
        }
    }
    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(parent_encodings.size());

    for (auto& parent_encoding : parent_encodings){
        auto [partial_solution_a, partial_solution_b] = best_combinations[parent_encoding];
        int domination_number = partial_solution_a->domination_number + partial_solution_b->domination_number - count_white_vertices(parent_encoding);
        std::vector<int> solution = partial_solution_a->solution->solution;
        for (int a : partial_solution_b->solution->solution){
            auto pos = lower_bound(solution.begin(), solution.end(), a);
            solution.insert(pos,a);
        }
        insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, solution, domination_number);
    }
    remove_all_entries_partial_solution(temp_child_partial_solution_a);
    remove_all_entries_partial_solution(child_partial_solution_b);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);
}

void TREEWIDTH_SOLVER::run_operation_introduce_edge(std::vector<uint>& bag, int endpoint_a, int endpoint_b){
    //find index of introduced vertex in the bag.
    int index_endpoint_a = find_index_in_bag(bag, endpoint_a);
    int index_endpoint_b = find_index_in_bag(bag, endpoint_b);

    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    std::vector<partial_solution> new_partial_solutions;
    new_partial_solutions.reserve(child_partial_solution.size());

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
            int manipulate_color_b = manipulate_color(index_endpoint_b, bag.size());
            std::uint64_t parent_encoding = child_encoding.encoding - (2 * manipulate_color_b);
            // first copy entry.
            insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, child_encoding.solution->solution, child_encoding.domination_number);
            continue;
        }
        if (color_endpoint_a == GRAY && color_endpoint_b == WHITE){
            int manipulate_color_a = manipulate_color(index_endpoint_a, bag.size());
            std::uint64_t parent_encoding = child_encoding.encoding - (2 * manipulate_color_a);
            // first copy entry.
            insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
            insert_entry_new_partial_solution(new_partial_solutions, parent_encoding, child_encoding.solution->solution, child_encoding.domination_number);
        }
        else {
            insert_entry_new_partial_solution(new_partial_solutions, child_encoding.encoding, child_encoding.solution->solution, child_encoding.domination_number);
        }
    }
    remove_all_entries_partial_solution(child_partial_solution);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);


}

void TREEWIDTH_SOLVER::run_operation_forget(std::vector<uint>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex){
    int index_forget_vertex = find_index_in_bag(bag, forget_vertex);

    //get previous partial solution.
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    std::vector<std::uint64_t> parent_encodings;
    parent_encodings.reserve(child_partial_solution.size() / 2);
    boost::unordered_map<std::uint64_t,partial_solution*> temp_partial_solutions; //parent encoding, child.

    for (auto& child_encoding : child_partial_solution){
        std::uint64_t parent_encoding = remove_color_at_index(child_encoding.encoding, index_forget_vertex);

        const int color = extract_bits(child_encoding.encoding, bag.size(), index_forget_vertex);
        if (color == GRAY) {
            continue;
        }
        if (color == BLACK) {
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
            continue;
        }
        if (color == WHITE) {
            if (auto it = temp_partial_solutions.find(parent_encoding); it == temp_partial_solutions.end()){
                temp_partial_solutions.insert({parent_encoding, &child_encoding});
                parent_encodings.push_back(parent_encoding);
            } else {
                if (child_encoding.domination_number < it->second->domination_number){
                    temp_partial_solutions[parent_encoding] = &child_encoding;
                }
            }
        }
    }
    std::vector<partial_solution> new_partial_solution;
    new_partial_solution.reserve(parent_encodings.size());

    for (auto& parent_encoding : parent_encodings){
        const auto best_child = temp_partial_solutions[parent_encoding];
        const int color = extract_bits(best_child->encoding, bag.size(), index_forget_vertex);
        if (color == BLACK){
            //no new introduces.
            insert_entry_new_partial_solution(new_partial_solution, parent_encoding, best_child->solution->solution, best_child->domination_number);
            continue;
        }
        if (color == WHITE) {
            std::vector<int> solution = best_child->solution->solution;
            auto pos = std::lower_bound(solution.begin(), solution.end(), forget_vertex);
            solution.insert(pos, forget_vertex);
            insert_entry_new_partial_solution(new_partial_solution, parent_encoding, solution, best_child->domination_number);
        }
    }
    remove_all_entries_partial_solution(child_partial_solution);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solution);
}

void TREEWIDTH_SOLVER::solve_root_vertex() {
    std::vector<uint>& bag_root_vertex = nice_tree_decomposition_ptr->nice_bags[nice_tree_decomposition_ptr->root_vertex].bag;
    std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

    int lowest_domination_number = INT_MAX;
    partial_solution* lowest_encoding = nullptr;

    for (auto& child : child_partial_solution){
        if (contains_no_gray(child.encoding)){
            if (lowest_domination_number < child.domination_number){
                lowest_domination_number = child.domination_number;
                lowest_encoding = &child;
            }
        }
    }
    if (lowest_encoding == nullptr){
        throw std::runtime_error("should never be null");
    }
    global_solution = lowest_encoding->solution->solution;
    std::vector<int> white_indices = get_white_indices(lowest_encoding->encoding, bag_root_vertex.size());
    for (int index : white_indices) {
        if (global_solution.size() == 0)
        {
            global_solution.push_back(bag_root_vertex[index]);
        } else
        {
            auto pos = std::lower_bound(global_solution.begin(), global_solution.end(), bag_root_vertex[index]);
            if (*pos == bag_root_vertex[index])
            {
                throw std::runtime_error(":(");
            }
            global_solution.insert(pos, bag_root_vertex[index]);
        }
    }

}

solution_struct::solution_struct(const std::vector<int>& solution) {
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

//set all GRAY to BLACK
std::uint64_t create_parent_join(const std::uint64_t encoding){
    const std::uint64_t mask = 0x5555555555555555;
    const std::uint64_t find_gray = ((encoding & (encoding >> 1) & mask));
    return (find_gray << 1) ^ encoding;
}

//makes all GRAY encodings BLACK and all BLACK encodings GRAY.
std::uint64_t create_compliment_encoding(const std::uint64_t encoding) {
    const std::uint64_t mask = 0x5555555555555555;
    const std::uint64_t find_gray = ((encoding & (encoding >> 1) & mask));
    const std::uint64_t find_black = ((encoding & (~encoding >> 1) & mask));
    return ((find_gray | find_black) << 1) ^ encoding;
}

//Helper function. Assumption: bag is ordered.
//either returns the index of the element, or the location where element should be inserted (to keep the bag sorted).
int find_index_in_bag(const std::vector<uint>& bag, const int element) {
    auto it = std::lower_bound(bag.begin(), bag.end(), element); // point to the first element equal or bigger than the element.

    return it - bag.begin(); //returns the index.
}

//Helper function.
//Can change the color of the vertex which has this index. (int manipulation).
std::uint64_t manipulate_color(const int index_vertex, const int bag_size) {
    const int power = bag_size - index_vertex;
    return 1 << (2 * power); //equivalent to 4^power
}

//Helper function.
std::uint64_t add_color_at_index(std::uint64_t encoding, int index) {
    const std::uint64_t mask = 0xFFFFFFFFFFFFFFFF << ((2 * index) + 2); // need 2 extra so (newly introduced color is empty (00)).
    return ((encoding ^ (encoding << 2)) & mask) ^ encoding;

}

//Helper function. removes 2 bits (a color) at the index of the bag.
std::uint64_t remove_color_at_index(const std::uint64_t encoding, const int index) {
    const std::uint64_t mask = 0xFFFFFFFFFFFFFFFF << (2 * index); // 0xF = 11.
    return ((encoding ^ (encoding >> 2) & mask)) ^ encoding;
}

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

