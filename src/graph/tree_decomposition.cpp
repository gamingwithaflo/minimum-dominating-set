#include "tree_decomposition.h"
//
#include <iostream>
#include <limits.h>
#include "../util/timer.h"
#include "../util/logger.h"
//
solution_struct_2::solution_struct_2(std::vector<int> sol) : ref_count(1), solution(sol) {}
//
TREE_DECOMPOSITION::TREE_DECOMPOSITION(std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition) : nice_tree_decomposition_ptr(std::move(nice_tree_decomposition)) {
	instruction_stack;
	partial_solution_stack;
	local_solution;
	global_solution;
}

//currently random root node. (preprocessing the root node could boost performance).
//int TREE_DECOMPOSITION::select_root_bag() {
//	//get vertices.
//	auto [vert_itt, vert_itt_end] = boost::vertices(graph_td);
//	for (;vert_itt < vert_itt_end; ++vert_itt) {
//		//get out degree.
//		int out_degree = boost::out_degree(*vert_itt, graph_td);
//
//		//potential root node.
//		if (out_degree == 1) {
//			return *vert_itt;
//		}
//	}
//	throw std::invalid_argument("There has to be at least one leaf vertex.");
//}

//void TREE_DECOMPOSITION::create_nice_tree_decomposition(adjacencyListBoost&  reduced_graph) {
//	auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_td);
//	int parent_index = root_vertex;
//	//root vertex is only adjacent to 1 vertex.
//	unfold_parent_vertex(root_vertex,bags[root_vertex], *itt, bags[*itt]);
//
//	//With Breath first traversel go through graph.
//	traverse_tree_decomposition(root_vertex, *itt);
//
//	introduce_all_edges_default(reduced_graph);
//
//	//introduce_all_edges(boost::edges(reduced_graph));
//}

//std::vector<std::pair<int, int>> findAllPairs(const std::vector<int>& bag) {
//	std::vector<std::pair<int, int>> res;
//	for (size_t i = 0; i < bag.size(); ++i) {
//		for (size_t j = i + 1; j < bag.size(); ++j) { // Avoid duplicate pairs
//			res.push_back(std::make_pair(bag[i], bag[j]));
//		}
//	}
//	return res;
//}
//
////find which vertices in the bag of the child are adjacent to the about to be forgotten vertex.
//std::vector<int> which_edges_must_be_introduced(adjacencyListBoost& original_graph, std::vector<int>bag_child, int forget_vertex) {
//	std::vector<int>must_introduce;
//	for (int end_point : bag_child) {
//		if (end_point != forget_vertex) {
//			auto [edge, exists] = boost::edge(forget_vertex, end_point, original_graph);
//			if (exists) {
//				must_introduce.push_back(end_point);
//			}
//		}
//	}
//	return must_introduce;
//}
//
//void TREE_DECOMPOSITION::introduce_all_edges_default(adjacencyListBoost& original_graph) {
//
//	 auto pairs = findAllPairs(nice_bags[root_vertex].bag);
//	 //introduce all vertices for the root vertex and update root_vertex.
//	 for (auto& [endpoint_a, endpoint_b] : pairs) {
//		 auto [edge, exists] = boost::edge(endpoint_a, endpoint_b, original_graph);
//		 if (exists) {
//			 vertex new_root = boost::add_vertex(graph_nice_td);
//			 boost::add_edge(root_vertex, new_root, graph_nice_td);
//			 nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, endpoint_a, endpoint_b, nice_bags[root_vertex].bag));
//			 root_vertex = new_root;
//		 }
//	 }
//
//	 //root vertex is adjacent to only one vertex.
//	 auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_nice_td);
//
//	std::queue<std::pair<int, int>> q;
//
//	q.emplace(*itt, root_vertex); // {child, parent}
//
//	 //(breath first traversal) find highest point where both endpoint_a and endpoint_b are present.
//	while (!q.empty()) {
//		auto [current_vertex, parent] = q.front();
//		q.pop();
//
//		std::vector<int>& bag_child = nice_bags[current_vertex].bag;
//		nice_bag& bag_parent = nice_bags[parent];
//
//		if (std::holds_alternative<operation_forget>(bag_parent.op)) {
//			operation_forget& op = std::get<operation_forget>(bag_parent.op);
//			int forget_vertex = op.vertex;
//
//			std::vector<int> must_introduce = which_edges_must_be_introduced(original_graph, bag_child, forget_vertex);
//
//			for (int end_point : must_introduce) {
//				boost::remove_edge(parent, current_vertex, graph_nice_td);
//				vertex introduce_edge = boost::add_vertex(graph_nice_td);
//				nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, end_point, forget_vertex, nice_bags[current_vertex].bag));
//				//keep sorted.
//				if (parent > current_vertex) {
//					boost::add_edge(current_vertex, introduce_edge, graph_nice_td);
//					boost::add_edge(parent, introduce_edge, graph_nice_td);
//				}
//				else {
//					boost::add_edge(parent, introduce_edge, graph_nice_td);
//					boost::add_edge(current_vertex, introduce_edge, graph_nice_td);
//				}
//				parent = introduce_edge;
//
//			}
//		}
//
//		auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
//		for (; itt != itt_end; ++itt) {
//			if (*itt != parent) {  // Avoid revisiting the parent
//				q.push({ *itt, current_vertex });
//			}
//		}
//	}
//}
//
////boost::source(edge_descriptor, graph) & target(edge_descriptor, graph).
//void TREE_DECOMPOSITION::introduce_all_edges(std::pair< edge_itt, edge_itt> edges_itterator) {
//	edge_itt edge_it = edges_itterator.first;
//	edge_itt edge_it_end = edges_itterator.second;
//	bool a_present;
//	bool b_present;
//
//	//do this for all edges.
//	for (;edge_it != edge_it_end; ++edge_it) {
//		//what we want to find.
//		int size_smallest_bag;
//		int index_smallest_bag;
//		int parent_smallest_bag;
//
//		//get both endpoints
//		auto endpoint_a = boost::source(*edge_it, graph_td);
//		auto endpoint_b = boost::target(*edge_it, graph_td);
//
//		std::vector<int>& root_vertex_bag = nice_bags[root_vertex].bag;
//
//		a_present = std::binary_search(root_vertex_bag.begin(), root_vertex_bag.end(), endpoint_a);
//		b_present = std::binary_search(root_vertex_bag.begin(), root_vertex_bag.end(), endpoint_b);
//
//		//introduce above root node. (and set new root vertex).
//		if (a_present && b_present) {
//			size_smallest_bag = nice_bags[root_vertex].bag.size();
//			index_smallest_bag = root_vertex;
//			parent_smallest_bag = -1; // has no parent.
//		}
//		else {
//			//root vertex should only have 1 adjacent vertex.
//			if (boost::out_degree(root_vertex, graph_nice_td) > 1) {
//				//
//			}
//			auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_nice_td);
//			std::queue<std::pair<int, int>> q; // {current_vertex, parent_vertex}
//
//			int parent;
//			int current_vertex;
//
//			q.push({ *itt, root_vertex });
//
//			//(breath first traversal) find highest point where both endpoint_a and endpoint_b are present.
//			while (!q.empty() && !(a_present && b_present)) {
//				std::tie(current_vertex, parent) = q.front();
//				q.pop();
//
//				nice_bag& curr_nice_bags = nice_bags[current_vertex];
//
//				if (std::holds_alternative<operation_forget>(curr_nice_bags.op)) {
//					operation_forget& op = std::get<operation_forget>(curr_nice_bags.op);
//					if (op.vertex == endpoint_a) a_present = true;
//					if (op.vertex == endpoint_b) b_present = true;
//				}
//
//				auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
//				for (; itt != itt_end; ++itt) {
//					if (*itt != parent) {  // Avoid revisiting the parent
//						q.push({ *itt, current_vertex });
//					}
//				}
//			}
//			//initialize smallest bag
//			auto [itt_curr, itt_curr_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
//			for (;itt_curr < itt_curr_end; ++itt_curr) {
//				//should be only one other bag besides its parent.
//				if (*itt_curr != parent) {
//					size_smallest_bag = nice_bags[*itt_curr].bag.size();
//					index_smallest_bag = *itt_curr;
//					parent_smallest_bag = current_vertex;
//				}
//			}
//		}
//		//keep track of the size and index of smallest bag you encountered & its parents index.
//
//		//If a bag introduces one of our endpoints stop the traversal down that branch.
//		std::queue<std::pair<int, int>> q; // {current_vertex, parent_vertex}
//
//		q.push({ index_smallest_bag, parent_smallest_bag });
//
//		while (!q.empty()) {
//			auto [current_vertex, parent] = q.front();
//			q.pop();
//
//			nice_bag& curr_nice_bags = nice_bags[current_vertex];
//
//			if (size_smallest_bag > curr_nice_bags.bag.size()) {
//				size_smallest_bag = curr_nice_bags.bag.size();
//				index_smallest_bag = current_vertex;
//				parent_smallest_bag = parent;
//			}
//
//			//if one of the vertices is introduced, then you do not have to explore this branch any further.
//			if (std::holds_alternative<operation_introduce>(curr_nice_bags.op)) {
//				operation_introduce& op = std::get<operation_introduce>(curr_nice_bags.op);
//				if (op.vertex == endpoint_a) continue;
//				if (op.vertex == endpoint_b) continue;
//			}
//
//			auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
//			for (; itt != itt_end; ++itt) {
//				if (*itt != parent) {
//					q.push({ *itt, current_vertex });
//				}
//			}
//		}
//		//add vertex in between index smallest bag & parent.
//		auto vertex = boost::add_vertex(graph_nice_td);
//		if (parent_smallest_bag == -1) {
//			boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
//			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, endpoint_a, endpoint_b, nice_bags[index_smallest_bag].bag));
//			root_vertex = vertex;
//		}
//		else {
//			//should never be the same index.
//			if (index_smallest_bag < parent_smallest_bag) {
//				boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
//				boost::add_edge(parent_smallest_bag, vertex, graph_nice_td);
//			}
//			else {
//				boost::add_edge(parent_smallest_bag, vertex, graph_nice_td);
//				boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
//			}
//			boost::remove_edge(index_smallest_bag, parent_smallest_bag, graph_nice_td);
//			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, endpoint_a, endpoint_b, nice_bags[index_smallest_bag].bag));
//		}
//	}
//}
//
//void TREE_DECOMPOSITION::traverse_tree_decomposition(int parent_index, vertex v) {
//	//we walk through the original graph, so changes to nice_graph_td will not effect your path.
//	auto [itt, itt_end] = boost::adjacent_vertices(v, graph_td);
//	int out_degree = boost::out_degree(v, graph_td);
//	--out_degree; //don't count parent.
//
//	//vertex is a leaf.
//	if (out_degree == 0) {
//		unfold_leaf_vertex(v);
//		return;
//	}
//
//	//parent vertex is either a introduce or forget node.
//	if (out_degree == 1) {
//		for (;itt < itt_end; ++itt) {
//			if (!(*itt == parent_index)) {
//
//				unfold_parent_vertex(v, bags[v], *itt, bags[*itt]);
//				traverse_tree_decomposition(v, *itt);
//			}
//		}
//	}
//	else {
//		nice_bags[v] = nice_bag(operation_enum::JOIN, bags[v]);
//		std::queue<int>children;
//		for (;itt < itt_end; ++itt) {
//			//push all children vertices on the queue.
//			if (!(*itt == parent_index)) {
//				boost::remove_edge(v, *itt, graph_nice_td);
//
//				vertex child = boost::add_vertex(graph_nice_td);
//
//				//add all edges to parent (v) and if it is not adjacent to v remove it.
//				boost::add_edge(v, child, graph_nice_td);
//
//				boost::add_edge(*itt, child, graph_nice_td);
//				children.push(child);
//
//				//put in a empty placeholder.
//				nice_bags.push_back(nice_bag());
//
//				//will fill the empty placeholder.
//				unfold_parent_vertex(child, bags[v], *itt, bags[*itt]);
//				traverse_tree_decomposition(v, *itt); // we need v, as that is in the original graph the parent.
//			}
//		}
//		// if queue is not empty. (child_a & child_b are not adjacent to v.
//		while (children.size() != 2) { // if it equals two, it already has been connected correctly.
//			int child_a = children.front();
//			children.pop();
//			int child_b = children.front();
//			children.pop();
//
//			boost::remove_edge(v, child_a, graph_nice_td);
//			boost::remove_edge(v, child_b, graph_nice_td);
//
//			vertex new_child = boost::add_vertex(graph_nice_td);
//			boost::add_edge(v, new_child, graph_nice_td);
//			//add edges, ensuring that the order is maintained.
//			if (child_a > child_b) {
//				boost::add_edge(child_b, new_child, graph_nice_td);
//				boost::add_edge(child_a, new_child, graph_nice_td);
//			}
//			else {
//				boost::add_edge(child_a, new_child, graph_nice_td);
//				boost::add_edge(child_b, new_child, graph_nice_td);
//			}
//			children.push(new_child);
//			nice_bags.push_back(nice_bag(operation_enum::JOIN, bags[v]));
//		}
//	}
//}
//
//void TREE_DECOMPOSITION::unfold_parent_vertex(int parent, std::vector<int>& bag_parent, int child, std::vector<int>& bag_child) {
//	int prev_vertex = child;
//
//	std::vector<int> acc_bag = bags[child];
//
//	// if bag_parent has child bag_child.
//	// Elements which are in bag_child but not in bag_parent -> serie of forget vertices.
//	// Elements which are in bag_parent but not in bag_child -> serie of introduce vertices.
//	std::vector<int>forget_vertices = find_non_overlapping_vertices(bag_parent, bag_child);
//	std::vector<int>introduce_vertices = find_non_overlapping_vertices(bag_child, bag_parent);
//
//	if (introduce_vertices.size() > 1 || (introduce_vertices.size() > 0 && forget_vertices.size() > 0)) {
//		//you will insert vertices so break up edge between parent and child.
//		boost::remove_edge(parent, child, graph_nice_td);
//
//		while (!forget_vertices.empty()) {
//			//get element for forget vertices.
//			int last_element = forget_vertices.back();
//			forget_vertices.pop_back();
//
//			//remove vertex from accumulated bag
//			if (auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element); it != acc_bag.end()) {
//				acc_bag.erase(it);
//			}
//
//			//create vertex and add edge.
//			auto forget_vertex = boost::add_vertex(graph_nice_td);
//			boost::add_edge(prev_vertex, forget_vertex, graph_nice_td);
//			nice_bags.push_back(nice_bag(operation_enum::FORGET, last_element, acc_bag));
//			prev_vertex = forget_vertex;
//		}
//
//		while (introduce_vertices.size() > 1) {
//			//get element from introduce_vertices.
//			int last_element = introduce_vertices.back();
//			introduce_vertices.pop_back();
//
//			//add element to accumulated bag.
//			auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
//			acc_bag.insert(it, last_element);
//
//			//create vertex and add edge.
//			auto introduce_vertex = boost::add_vertex(graph_nice_td);
//			boost::add_edge(prev_vertex, introduce_vertex, graph_nice_td);
//			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE, last_element, acc_bag));
//			prev_vertex = introduce_vertex;
//		}
//
//		//introduce_vertices.size() == 1.
//		int last_element = introduce_vertices.back();
//
//		//add edge.
//		boost::add_edge(prev_vertex, parent, graph_nice_td);
//		nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, bag_parent);
//	}
//	else if (introduce_vertices.size() == 1) {
//		//you will insert vertices so break up edge between parent and child.
//		int last_element = introduce_vertices.back();
//
//		nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, bag_parent);
//	}
//	else if (forget_vertices.size() == 1) {
//		//you will insert vertices so break up edge between parent and child.
//		int last_element = forget_vertices.back();
//
//		nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
//	}
//	else if (forget_vertices.size() > 1) {while (forget_vertices.size() > 1) {
//			//get element from forget_vertices.
//			int last_element = forget_vertices.back();
//			forget_vertices.pop_back();
//
//			//remove vertex from accumulated bag
//			auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
//			if (it != acc_bag.end()) {
//				acc_bag.erase(it);
//			}
//
//			//create vertex and add edge.
//			auto forget_vertex = boost::add_vertex(graph_nice_td);
//			boost::add_edge(prev_vertex, forget_vertex, graph_nice_td);
//			nice_bags.push_back(nice_bag(operation_enum::FORGET, last_element, acc_bag));
//			prev_vertex = forget_vertex;
//		}
//		boost::remove_edge(parent, child, graph_nice_td);
//
//		//forget_vertices == 1
//		int last_element = forget_vertices.back();
//
//		//add edge.
//		boost::add_edge(prev_vertex, parent, graph_nice_td);
//		nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
//	}
//	else {
//		throw std::invalid_argument("something unforseen");
//	}
//}
//
////Get all elements which are in b, but not in a.
//std::vector<int> find_non_overlapping_vertices(const std::vector<int>& a, const std::vector<int>& b) {
//	std::vector<int> result;
//	size_t i = 0, j = 0;
//
//	while (j < b.size()) {
//		if (i < a.size() && a[i] < b[j]) {
//			//move pointer forward.
//			i++;
//		}
//		else if (i < a.size() && a[i] == b[j]) {
//			//element exists in both, move both pointers.
//			i++;
//			j++;
//		}
//		else {
//			//Element is in b but not in a.
//			result.push_back(b[j]);
//			j++;
//		}
//	}
//	return result;
//}
//
//void TREE_DECOMPOSITION::unfold_leaf_vertex(int vertex) {
//	std::vector<int> bag = bags[vertex];
//
//	//Create nice_bag on original place.
//	int lastElement = bag.back();
//	nice_bags[vertex] = nice_bag(operation_enum::INTRODUCE, lastElement, bag);
//	bag.pop_back();
//
//	int prev_vertex = vertex;
//	while (!bag.empty()) {
//		lastElement = bag.back();
//
//		//Create new vertex.
//		auto v = boost::add_vertex(graph_nice_td);
//		nice_bags.push_back(nice_bag(operation_enum::INTRODUCE, lastElement, bag));
//		auto e = boost::add_edge(prev_vertex, v, graph_nice_td);
//		prev_vertex = v;
//		bag.pop_back();
//	}
//	//create an empty leaf node.
//	auto leaf_v = boost::add_vertex(graph_nice_td);
//	//bag should be empty.
//	nice_bags.push_back(nice_bag(operation_enum::LEAF, bag));
//	auto leaf_e = boost::add_edge(prev_vertex, leaf_v, graph_nice_td);
//}
//
void TREE_DECOMPOSITION::fill_instruction_stack() {
	depth_first_search(nice_tree_decomposition_ptr->root_vertex, -1);
}

void TREE_DECOMPOSITION::depth_first_search(int start, int parent) {
	//depth first traversel. (the only splits should be join vertices).
	instruction_stack.push(&nice_tree_decomposition_ptr->nice_bags[start]);
	auto [itt, itt_end] = boost::out_edges(start, nice_tree_decomposition_ptr->graph_nice_tree_decomposition);
	for (; itt != itt_end; ++itt) {
		int depth = boost::target(*itt, nice_tree_decomposition_ptr->graph_nice_tree_decomposition);
		depth_first_search(depth, start);
	}
}

void TREE_DECOMPOSITION::run_instruction_stack(std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex) {

	while (!instruction_stack.empty()) {
		//get top instruction from the stack. (is a pointer).
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

void TREE_DECOMPOSITION::insert_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int,solution_struct_2*>>& partial_solution,
													   std::uint64_t encoding,
													   std::vector<int> key, int size) {

	auto it = local_solution.find(key);

	//if key is not in local_solution, create & insert it.
	if (it == local_solution.end()) {
		//emplace will create it within & will not copy it.
		auto [new_it, succes] = local_solution.emplace(key, solution_struct_2(key));

		// Store a pointer to the newly created solution_struct in partial_solution.
		partial_solution.insert({ encoding ,std::make_pair(size, &new_it->second) });
	}
	else {
		//it is a key, value pair.
		it->second.ref_count++;
		partial_solution.insert({ encoding ,std::make_pair(size, &it->second) });
	}
}

void TREE_DECOMPOSITION::remove_all_entry_partial_solution(std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>>& child_partial_solution_vector) {

	for (auto& [size, sol_struct] : child_partial_solution_vector) {
		//check for null_pointer.
		if (sol_struct.second) {
			//decrease ref counter. (as child partial solutions will be removed.
			sol_struct.second->ref_count--;
			if (sol_struct.second->ref_count == 0) {
				//remove from local_solution if the ref_count reaches 0.
				local_solution.erase(sol_struct.second->solution); // use key key to remove the element.
			}
		}
	}
}



int find_index_in_bag(std::vector<uint>& bag, int element) {
	auto it = std::lower_bound(bag.begin(), bag.end(), element); // point to the first element equal or bigger than the element.

	return it - bag.begin(); //returns the index.
}

int find_index_in_bag_edge(std::vector<uint>& bag, int element) {
	auto it = std::lower_bound(bag.begin(), bag.end(), element); // point to the first element equal or bigger than the element.
	//temporary check
	if (*it != element)
	{
		throw std::runtime_error("Element not found");
	}

	return it - bag.begin(); //returns the index.
}

int extract_bits_2(std::uint64_t encoding, int size_bag, int pos) {
	return (encoding >> (2 * (size_bag - 1 - pos))) & 0b11;
}

//(2-bit representation of colors of vertices). 0b01 -> black, 0b11 -> gray, 0b10 -> white.
const int COLORS[] = { 0b01, 0b11, 0b10 };
const int NUM_COLORS = 3;


void TREE_DECOMPOSITION::run_operation_leaf() {
	timer t_operation_leaf;
	//Push an empty partial solution onto the partial solution stack.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> empty_partial_solution;
	partial_solution_stack.push(empty_partial_solution);
	Logger::execution_time_leaf += t_operation_leaf.count();
}

void generate_encoding_forget(int n, std::uint64_t coloring, std::uint64_t child_coloring, int position, int index_forget, std::vector<std::pair<std::uint64_t, std::uint64_t>>& results) {
	//as the child_coloring is one size bigger than the original bag.
	if (position == (n + 1)) {
		results.push_back(std::make_pair(coloring, child_coloring));
		return;
	}

	if (position == index_forget) {
		generate_encoding_forget(n, coloring, (child_coloring << 2) | COLORS[2], position + 1, index_forget, results);
	}
	else {
		for (int i = 0; i < NUM_COLORS; ++i) {
			generate_encoding_forget(n, (coloring << 2) | COLORS[i], (child_coloring << 2) | COLORS[i], position + 1, index_forget, results);
		}
	}
}

std::vector<std::pair<std::uint64_t, std::uint64_t>> generate_all_encoding_forget(int n, int index_forget) {
	std::vector<std::pair<std::uint64_t, std::uint64_t>> results;
	generate_encoding_forget(n, 0, 0, 0, index_forget, results);
	return results;
}

void TREE_DECOMPOSITION::run_operation_forget(std::vector<uint>& bag, int forget_vertex, std::vector<int>& excluded, std::unordered_map<int, int>& newToOldIndex) {
	timer t_operation_forget;
	//find index of introduced vertex in the bag.
	int index_forget_vertex = find_index_in_bag(bag, forget_vertex);

	//create an empty partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> partial_solution;

	//get previous childs partial solution.
	std::unordered_map<std::uint64_t,std::pair<int, solution_struct_2*>> child_partial_solution = partial_solution_stack.top();
	partial_solution_stack.pop();

	if (bag.size() == 0)
	{
		//throw std::runtime_error("The graph is empty");
	}

	//generate all forget encodings.
	std::vector<std::pair<std::uint64_t, std::uint64_t>> encoding_vector = generate_all_encoding_forget(bag.size(), index_forget_vertex);
	for (const auto& [encoding, child_encoding] : encoding_vector) {
		//Does this actually work?
		int power = bag.size() - index_forget_vertex;
		int decrease_color = 1 << (2 * power); // equivalent to 4^power
		std::uint64_t child_encoding_white = child_encoding;
		std::uint64_t child_encoding_black = child_encoding - decrease_color;
		//if childs partial solution is infinite. Whatever you do, when introducing a vertex its partial solution is also infinite.
		auto ptr_white = child_partial_solution.find(child_encoding_white);
		auto ptr_black = child_partial_solution.find(child_encoding_black);

		if (excluded[newToOldIndex[forget_vertex]] == 1) {
			//if it is black but rest of encoding is not valid, no deal.
			if (ptr_black == child_partial_solution.end()) {
				continue;
			}
			std::vector<int>solution = child_partial_solution[child_encoding_black].second->solution;
			insert_entry_partial_solution(partial_solution, encoding, solution, child_partial_solution[child_encoding_black].first);
			continue;
		}


		if (ptr_white == child_partial_solution.end() && ptr_black == child_partial_solution.end()) {
			//both encodings lead to infinite values. (dont introduce it).
			continue;
		}
		//if white is only infinte.
		if (ptr_white == child_partial_solution.end()) {
				insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding_black].second->solution, child_partial_solution[child_encoding_black].first);
				continue;
		}
		//if black is only infinite.
		if (ptr_black == child_partial_solution.end()) {
			std::vector<int>solution = child_partial_solution[child_encoding_white].second->solution;
			auto pos = std::lower_bound(solution.begin(), solution.end(), forget_vertex);
			solution.insert(pos, forget_vertex);
			insert_entry_partial_solution(partial_solution, encoding, solution, child_partial_solution[child_encoding_white].first);
			continue;
		}
		//both not infinite original.
		if (child_partial_solution[child_encoding_white].first > child_partial_solution[child_encoding_black].first) {
			insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding_black].second->solution, child_partial_solution[child_encoding_black].first);

		}
		else {
			std::vector<int> solution = child_partial_solution[child_encoding_white].second->solution;
			auto pos = std::lower_bound(solution.begin(), solution.end(), forget_vertex);
			solution.insert(pos, forget_vertex);
			insert_entry_partial_solution(partial_solution, encoding, solution, child_partial_solution[child_encoding_white].first);
		}

	}
	remove_all_entry_partial_solution(child_partial_solution);
	partial_solution_stack.push(partial_solution);
	Logger::execution_time_forget += t_operation_forget.count();
}

std::vector<std::uint64_t> generate_all_encoding(int n) {
	std::vector<std::uint64_t> results;
	generate_encoding(n, 0, 0, results);
	return results;
}

void generate_encoding(int n, int coloring, int position, std::vector<std::uint64_t>& results) {
	if (position == n) {
		results.push_back(coloring);
		return;
	}
	for (int i = 0; i < NUM_COLORS; ++i) {
		generate_encoding(n, (coloring << 2) | COLORS[i], position + 1, results);
	}
}

std::vector<std::pair<std::uint64_t, std::uint64_t>> generate_all_encoding_introduce(int n, int index_introduced) {
	std::vector<std::pair<std::uint64_t, std::uint64_t>> results;
	generate_encoding_introduce(n, 0, 0, 0, index_introduced, results);
	return results;
}

void generate_encoding_introduce(int n, std::uint64_t coloring, std::uint64_t child_coloring, int position, int index_introduced, std::vector<std::pair<std::uint64_t, std::uint64_t>>& results) {
	if (position == n) {
		results.push_back(std::make_pair(coloring, child_coloring));
		return;
	}

	for (int i = 0; i < NUM_COLORS; ++i) {
		if (position == index_introduced) {
			generate_encoding_introduce(n, (coloring << 2) | COLORS[i], child_coloring, position + 1, index_introduced, results);
		}
		else {
			generate_encoding_introduce(n, (coloring << 2) | COLORS[i], (child_coloring << 2) | COLORS[i], position + 1, index_introduced, results);
		}
	}
}

void TREE_DECOMPOSITION::run_operation_introduce(std::vector<uint>& bag, int introduced_vertex, std::vector<int>& dominated, std::vector<int>&excluded, std::unordered_map<int,int>& newToOldIndex) {
	timer t_operation_introduce;
	//find index of introduced vertex in the bag.
	int index_introduced_vertex = find_index_in_bag_edge(bag, introduced_vertex);

	//create an empty partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> partial_solution;

	//get previous childs partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> child_partial_solution = partial_solution_stack.top();
	partial_solution_stack.pop();

	// create all possible coloring pairs for introduced operation.
	if (child_partial_solution.empty()) {
		std::vector<std::uint64_t> encoding_vector = generate_all_encoding(bag.size());
		for (uint64_t encoding : encoding_vector) {
			//gray.
			if (encoding == 3) {
				//cant do this.
				// if (dominated[newToOldIndex[introduced_vertex]] == 1) {
				// 	continue;
				// }
				insert_entry_partial_solution(partial_solution, encoding, {}, 0);
				continue;
			}
			// white.
			if (encoding == 2) {
				//if vertex is excluded, not allow white.
				// if (excluded[newToOldIndex[introduced_vertex]] == 1)
				// {
				// 	continue;
				// }
				insert_entry_partial_solution(partial_solution, encoding, {}, 1);
				continue;
			}
			// black.
			if (encoding == 1) {
				//if vertex is dominated, not taking it does not cost anything.
				if (dominated[newToOldIndex[introduced_vertex]] == 1) {
					insert_entry_partial_solution(partial_solution, encoding, {}, 0);
					continue;
				}
				else {
					//we dont want to introduce infinite values.
					//insert_entry_partial_solution(partial_solution, encoding, {}, INT_MAX);
					continue;
				}
			}
		}
		partial_solution_stack.push(partial_solution);
	}
	else {
		std::vector<std::pair<std::uint64_t, std::uint64_t>> encoding_vector = generate_all_encoding_introduce(bag.size(), index_introduced_vertex);

		for (const auto& [encoding, child_encoding] : encoding_vector) {
			//if childs partial solution is infinite. Whatever you do, when introducing a vertex its partial solution is also infinite.
			if (child_partial_solution.find(child_encoding) == child_partial_solution.end()) {
				//insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, INT_MAX);
				continue;
			}

			int color = extract_bits_2(encoding, bag.size(), index_introduced_vertex);
			//gray.
			if (color == 3) {
				// if (dominated[newToOldIndex[introduced_vertex]] == 1) {
				// 	continue;
				// }
				int domination_number = 0 + child_partial_solution[child_encoding].first;
				insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number);
				continue;
			}
			//white.
			if (color == 2) {
					int domination_number = 1 + child_partial_solution[child_encoding].first;
					insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number);
					continue;
			}
			//black.
			if (color == 1) {
				// if vertex is dominated doesnt cost anything to introduce it.
				if (dominated[newToOldIndex[introduced_vertex]] == 1) {
					int domination_number = 0 + child_partial_solution[child_encoding].first;
					insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number);
				}
				else {
					//int domination_number = INT_MAX;
					//insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number);
					continue;
				}
			}
		}
		//remove all references of child_partial_solution as it will be removed after this.
		remove_all_entry_partial_solution(child_partial_solution);
		partial_solution_stack.push(partial_solution);
		Logger::execution_time_introduce += t_operation_introduce.count();
	}
}

void generate_encoding_join(int n, std::uint64_t coloring, int count_white, int position,
							std::vector<std::pair<std::uint64_t, uint64_t>>& coloring_child,
							std::vector< std::vector<std::pair<std::uint64_t, uint64_t>>>& coloring_child_vector,
							std::vector<std::uint64_t>& coloring_vector,
							std::vector<int>& number_of_ones) {
	//base case: If we reached the max size of the bag. (and the encoding.
	if (position == n) {
		coloring_vector.push_back(coloring);
		number_of_ones.push_back(count_white);
		coloring_child_vector.push_back(coloring_child);
		return;
	}

	for (int i = 0; i < NUM_COLORS; ++i) {
		std::uint64_t new_coloring = (coloring << 2) | COLORS[i];

		//if color is white.
		if (i == 2) {

			for (auto& pair : coloring_child) {
				pair.first = (pair.first << 2) | COLORS[i];
				pair.second = (pair.second << 2) | COLORS[i];
			}
			generate_encoding_join(n, new_coloring, count_white + 1, position + 1, coloring_child, coloring_child_vector, coloring_vector, number_of_ones);
		}
		//if color is gray.
		else if (i == 1) {
			std::vector<std::pair<std::uint64_t, uint64_t>> new_coloring_child = coloring_child;  // Create a new copy
			for (auto& pair : new_coloring_child) {
				pair.first = (pair.first << 2) | COLORS[i];
				pair.second = (pair.second << 2) | COLORS[i];
			}
			generate_encoding_join(n, new_coloring, count_white, (position + 1), new_coloring_child, coloring_child_vector, coloring_vector, number_of_ones);
		}
		//if color is black.
		else {
			std::vector<std::pair<std::uint64_t, uint64_t>> new_coloring_child = coloring_child;
			std::vector<std::pair<std::uint64_t, uint64_t>> temp_coloring_child;
			for (auto& pair : new_coloring_child) {
				//Question: does this only happen at the first divide?
				if (pair.first == pair.second) {
					pair.first = (pair.first << 2) | COLORS[0]; // Add black
					pair.second = (pair.second << 2) | COLORS[1]; // Add white
				}
				else {
					std::pair<std::uint64_t, std::uint64_t> new_pair = pair;

					pair.first = (pair.first << 2) | COLORS[0]; // Add black
					pair.second = (pair.second << 2) | COLORS[1]; // Add white

					new_pair.first = (new_pair.first << 2) | COLORS[1]; // Add white
					new_pair.second = (new_pair.second << 2) | COLORS[0]; // Add white

					temp_coloring_child.push_back(new_pair);
				}
			}
			if (!temp_coloring_child.empty()) {
				new_coloring_child.insert(new_coloring_child.end(), temp_coloring_child.begin(), temp_coloring_child.end());
			}
			generate_encoding_join(n, new_coloring, count_white, (position + 1), new_coloring_child, coloring_child_vector, coloring_vector, number_of_ones);
		}
	}
}

//result is given through the parameters.
void generate_all_encoding_join(int n,
								std::vector< std::vector<std::pair<std::uint64_t, uint64_t>>>& coloring_child_vector,
								std::vector<std::uint64_t>& coloring_vector,
								std::vector<int>& number_of_ones) {
	std::vector<std::pair<std::uint64_t, uint64_t>> empty_coloring_child = { {0, 0} };
	generate_encoding_join(n, 0, 0, 0, empty_coloring_child, coloring_child_vector, coloring_vector, number_of_ones);
}


void TREE_DECOMPOSITION::run_operation_join(std::vector<uint>& bag) {
	timer t_operation_join;
	//create an empty partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> partial_solution;
	if (bag.size() == 0)
	{
		std::cout << "not really fun" << std::endl;
	}
	//get previous childs partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> child_partial_solution_a = partial_solution_stack.top();
	partial_solution_stack.pop();
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> child_partial_solution_b = partial_solution_stack.top();
	partial_solution_stack.pop();

	//Create all encodings & child encodings.
	std::vector< std::vector<std::pair<std::uint64_t, uint64_t>>> coloring_child_vector;
	std::vector<std::uint64_t> encoding_vector;
	std::vector<int> number_of_ones;

	generate_all_encoding_join(bag.size(), coloring_child_vector, encoding_vector, number_of_ones);

	for (int i = 0; i < encoding_vector.size(); ++i) {
		int lowest_pair_cost = INT_MAX;
		std::pair<uint64_t, uint64_t> lowest_pair;
		bool is_result_a_lowest; //Store the orientation.

		auto& vector_pair_childern = coloring_child_vector[i];

		for (auto& pair : vector_pair_childern) {
			int result_a;
			int result_b;
			if (child_partial_solution_a.find(pair.first) == child_partial_solution_a.end() || child_partial_solution_b.find(pair.second) == child_partial_solution_b.end()){
				result_a = INT_MAX;
			}
			else {
				result_a = child_partial_solution_a[pair.first].first + child_partial_solution_b[pair.second].first - number_of_ones[i];
			}
			if (child_partial_solution_a.find(pair.second) == child_partial_solution_a.end() || child_partial_solution_b.find(pair.first) == child_partial_solution_b.end()) {
				result_b = INT_MAX;
			}
			else {
				result_b = child_partial_solution_a[pair.second].first + child_partial_solution_b[pair.first].first - number_of_ones[i];
			}

			// Get the minimum of the two results
			int lowest = (result_a < result_b) ? result_a : result_b;

			// if we find a lower cost, update lowest_pair_cost.
			if (lowest_pair_cost > lowest) {
				lowest_pair_cost = lowest;
				lowest_pair = pair;
				is_result_a_lowest = (result_a < result_b);
			}

			// Early exit optimization if lowest_pair_cost is 0.
			if (lowest_pair_cost == 0) {
				break;
			}
		}

		//Insert the result into the partial solution.
		std::vector<int> solution;
		if (lowest_pair_cost == INT_MAX) {
			{
				//solution is already empty & lowest_pair_cost is INT_MAX
				continue;

				//we dont want infinite encodings in the partial solution.
			}
		}
		if (is_result_a_lowest) {
			solution = child_partial_solution_a[lowest_pair.first].second->solution;
			for (int a : child_partial_solution_b[lowest_pair.second].second->solution) {
				//should never overlap. (this way it will always remain sorted).
				auto pos = lower_bound(solution.begin(), solution.end(), a);
				solution.insert(pos, a);
			}
		}
		else {
			solution = child_partial_solution_a[lowest_pair.second].second->solution;
			for (int a : child_partial_solution_b[lowest_pair.first].second->solution) {
				//should never overlap. (this way it will always remain sorted).
				auto pos = lower_bound(solution.begin(), solution.end(), a);
				solution.insert(pos, a);
			}
		}
		insert_entry_partial_solution(partial_solution, encoding_vector[i], solution, lowest_pair_cost);
	}
	remove_all_entry_partial_solution(child_partial_solution_a);
	remove_all_entry_partial_solution(child_partial_solution_b);
	partial_solution_stack.push(partial_solution);
	Logger::execution_time_join += t_operation_join.count();
}

void TREE_DECOMPOSITION::run_operation_introduce_edge(std::vector<uint>& bag, int endpoint_a, int endpoint_b) {
	timer t_operation_intoduce_edge;
	//find index of introduced vertex in the bag.
	int index_endpoint_a = find_index_in_bag_edge(bag, endpoint_a);
	int index_endpoint_b = find_index_in_bag_edge(bag, endpoint_b);

	//create an empty partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> partial_solution;

	//get previous childs partial solution.
	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> child_partial_solution = partial_solution_stack.top();
	partial_solution_stack.pop();

	std::vector<std::uint64_t> encoding_vector = generate_all_encoding(bag.size());

	for (const auto& encoding : encoding_vector){
		int color_endpoint_a = extract_bits_2(encoding, bag.size(), index_endpoint_a);
		int color_endpoint_b = extract_bits_2(encoding, bag.size(), index_endpoint_b);

		//endpoint a = white, endpoint b = black.
		if (color_endpoint_a == 2 && color_endpoint_b == 1) {
			int power = (bag.size() - 1) - index_endpoint_b;
			int increase_color = 1 << (2 * power); // equivalent to 4^power
			std::uint64_t child_encoding = encoding + (2 * increase_color); // go from black to gray.

			if (child_partial_solution.find(child_encoding) != child_partial_solution.end()) {
				int domination_number_child = child_partial_solution[child_encoding].first;
				insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number_child);
			} else
			{
				//never run.
				// if (child_partial_solution.find(encoding) != child_partial_solution.end()) {
				// 	int domination_number = child_partial_solution[encoding].first;
				// 	insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[encoding].second->solution, domination_number);
				// }
				continue;
			}
		}
		//endpoint a = black, endpoint b = white.
		else if (color_endpoint_a == 1 && color_endpoint_b == 2){
			int power = (bag.size() - 1) - index_endpoint_a;
			int increase_color = 1 << (2 * power); // equivalent to 4^power
			std::uint64_t child_encoding = encoding + (2 * increase_color); // go from black to gray.

			if (child_partial_solution.find(child_encoding) != child_partial_solution.end()) {
				int domination_number = child_partial_solution[child_encoding].first;
				insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[child_encoding].second->solution, domination_number);
			} else
			{
				//never run.
				// if (child_partial_solution.find(encoding) != child_partial_solution.end()) {
				// 	int domination_number = child_partial_solution[encoding].first;
				// 	insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[encoding].second->solution, domination_number);
				// }
				continue;
			}
		}
		// any other combination.
		else {
			if (child_partial_solution.find(encoding) != child_partial_solution.end()) {
				int domination_number = child_partial_solution[encoding].first;
				insert_entry_partial_solution(partial_solution, encoding, child_partial_solution[encoding].second->solution, domination_number);
			} else
			{
				continue;
			}
		}
	}
	remove_all_entry_partial_solution(child_partial_solution);
	partial_solution_stack.push(partial_solution);
	Logger::execution_time_introduce_edge += t_operation_intoduce_edge.count();
}

bool contains_no_gray_2(std::uint64_t encoding) {
	// Create a mask that isolates all even-positioned bit-pairs
	std::uint64_t mask = 0x5555555555555555; // 0b01 repeated (ensures checking pairs)

	// Check if value has any '11' pairs
	return (encoding & (encoding >> 1) & mask) == 0;
}

int count_white_vertices_2(std::uint64_t encoding) {
	uint64_t mask = 0x5555555555555555; // 0b01 repeated (ensures checking pairs)
	return __builtin_popcountll((~encoding) & (encoding >> 1) & mask); // ~ = bitflip, & = AND. & popcountll counts the number of 1 bits.
}

std::vector<int> get_white_indices_2(uint64_t encoding, int num_of_pairs) {
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

void TREE_DECOMPOSITION::solve_root_vertex() {
	std::vector<uint>& bag_root_vertex = nice_tree_decomposition_ptr->nice_bags[nice_tree_decomposition_ptr->root_vertex].bag;

	std::unordered_map<std::uint64_t, std::pair<int, solution_struct_2*>> solution = partial_solution_stack.top();
	partial_solution_stack.pop();

	int lowest_domination_number = INT_MAX;
	uint64_t lowest_encoding;

	for (auto& [encoding, sol] : solution) {
		if (contains_no_gray_2(encoding)) {
			if (lowest_domination_number > sol.first) {
				lowest_domination_number = sol.first;
				lowest_encoding = encoding;
			}
		}
	}
	//combine solution_struct_solution with the corresponding ones of the current bag.
	auto& [domination_number, ptr_local_solution] = solution[lowest_encoding];
	//should be sorted.
	global_solution = ptr_local_solution->solution;
	std::vector<int> white_indices = get_white_indices_2(lowest_encoding, bag_root_vertex.size());
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

//operation::operation(operation_enum type) : opp(type) {};
//
//operation_join::operation_join() : operation(operation_enum::JOIN) {};
//
//operation_leaf::operation_leaf() : operation(operation_enum::LEAF) {};
//
//operation_forget::operation_forget(int v) : operation(operation_enum::FORGET), vertex(v) {};
//
//operation_introduce::operation_introduce(int v) : operation(operation_enum::INTRODUCE), vertex(v) {};
//
//operation_introduce_edge::operation_introduce_edge(int v, int w) : operation(operation_enum::INTRODUCE_EDGE), endpoint_a(v), endpoint_b(w) {};
//
////placeholder.
//nice_bag::nice_bag() {};
//
//
//
//nice_bag::nice_bag(operation_enum operation, std::vector<int>bag_input) {
//	bag = bag_input;
//
//	switch (operation) {
//		case operation_enum::LEAF:
//			op = operation_leaf();
//			break;
//		case operation_enum::JOIN:
//			op = operation_join();
//			break;
//		default:
//			throw std::invalid_argument("wrong operator (or wrong function constructor)");
//	}
//}
//
//nice_bag::nice_bag(operation_enum operation, int v, std::vector<int>bag_input) {
//	bag = bag_input;
//
//	switch (operation) {
//		case operation_enum::FORGET:
//			op = operation_forget(v);
//			break;
//		case operation_enum::INTRODUCE:
//			op = operation_introduce(v);
//			break;
//		default:
//			throw std::invalid_argument("wrong operator (or wrong function constructor)");
//	}
//}
//
//nice_bag::nice_bag(operation_enum operation, int v, int w, std::vector<int>bag_input) {
//	bag = bag_input;
//
//	switch (operation) {
//		case operation_enum::INTRODUCE_EDGE:
//			op = operation_introduce_edge(v, w);
//			break;
//		default:
//			throw std::invalid_argument("wrong operator (or wrong function constructor)");
//
//	}
//}