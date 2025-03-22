#include "treewidth_solver.h"

TREEWIDTH_SOLVER::TREEWIDTH_SOLVER(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex) {

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

void TREEWIDTH_SOLVER::run_operation_introduce(std::vector<int>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex){
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

void TREEWIDTH_SOLVER::run_operation_join(std::vector<int>& bag){

}

void TREEWIDTH_SOLVER::run_operation_introduce_edge(std::vector<int>& bag, int endpoint_a, int endpoint_b){
    //find index of introduced vertex in the bag.
    int index_endpoint_a = find_index_in_bag(bag, endpoint_a);
    int index_endpoint_b = find_index_in_bag(bag, endpoint_b);

    const std::vector<partial_solution>& child_partial_solution = partial_solution_stack.top();

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
    remove_all_entries_partial_solution(new_partial_solutions);
    partial_solution_stack.pop();
    partial_solution_stack.push(new_partial_solutions);


}

void TREEWIDTH_SOLVER::run_operation_forget(std::vector<int>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex){
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

forget_info::forget_info(bool is_white, int domination_number, int index_solution){
    this->is_white = is_white;
    this->domination_number = domination_number;
    this->index_solution = index_solution;
}



//Helper function. Assumption: bag is ordered.
//either returns the index of the element, or the location where element should be inserted (to keep the bag sorted).
int find_index_in_bag(const std::vector<int>& bag, const int element) {
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

