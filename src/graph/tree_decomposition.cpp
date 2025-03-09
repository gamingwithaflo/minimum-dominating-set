
#include "tree_decomposition.h"
#include <limits.h>

TREE_DECOMPOSITION::TREE_DECOMPOSITION(std::vector<std::vector<int>> bags_input, adjacencyListBoost g, int treewidth_input) {
	graph_td = g;
	graph_nice_td = g;
	bags = bags_input;
	nice_bags.resize(bags.size());
	treewidth = treewidth_input;
	root_vertex = select_root_bag(); 
	instruction_stack;
	partial_solution_stack;
}

//currently random root node. (preprocessing the root node could boost performance).
int TREE_DECOMPOSITION::select_root_bag() {
	//get vertices.
	auto [vert_itt, vert_itt_end] = boost::vertices(graph_td);
	for (;vert_itt < vert_itt_end; ++vert_itt) {
		//get out degree.
		int out_degree = boost::out_degree(*vert_itt, graph_td);

		//potential root node.
		if (out_degree == 1) {
			return *vert_itt;
		}
	}
	throw std::invalid_argument("There has to be at least one leaf vertex.");
}

void TREE_DECOMPOSITION::create_nice_tree_decomposition(std::pair< edge_itt, edge_itt> edges_itterator) {
	auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_td);
	int parent_index = root_vertex;
	//root vertex is only adjacent to 1 vertex.
	unfold_parent_vertex(root_vertex,bags[root_vertex], *itt, bags[*itt]);

	//With Breath first traversel go through graph.
	traverse_tree_decomposition(root_vertex, *itt);

	introduce_all_edges(edges_itterator);
}

//boost::source(edge_descriptor, graph) & target(edge_descriptor, graph).
void TREE_DECOMPOSITION::introduce_all_edges(std::pair< edge_itt, edge_itt> edges_itterator) {
	edge_itt edge_it = edges_itterator.first;
	edge_itt edge_it_end = edges_itterator.second;
	bool a_present;
	bool b_present;

	//do this for all edges.
	for (;edge_it != edge_it_end; ++edge_it) {
		//what we want to find.
		int size_smallest_bag;
		int index_smallest_bag;
		int parent_smallest_bag;

		//get both endpoints
		auto endpoint_a = boost::source(*edge_it, graph_td);
		auto endpoint_b = boost::target(*edge_it, graph_td);

		std::vector<int>& root_vertex_bag = nice_bags[root_vertex].bag;

		a_present = std::binary_search(root_vertex_bag.begin(), root_vertex_bag.end(), endpoint_a);
		b_present = std::binary_search(root_vertex_bag.begin(), root_vertex_bag.end(), endpoint_b);

		//introduce above root node. (and set new root vertex).
		if (a_present && b_present) {
			size_smallest_bag = nice_bags[root_vertex].bag.size();
			index_smallest_bag = root_vertex;
			parent_smallest_bag = -1; // has no parent.
		}
		else {
			//root vertex should only have 1 adjacent vertex.
			auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_nice_td);
			std::queue<std::pair<int, int>> q; // {current_vertex, parent_vertex}

			int parent;
			int current_vertex;

			q.push({ *itt, root_vertex });

			//(breath first traversal) find highest point where both endpoint_a and endpoint_b are present.
			while (!q.empty() && !(a_present && b_present)) {
				std::tie(current_vertex, parent) = q.front();
				q.pop();

				nice_bag& curr_nice_bags = nice_bags[current_vertex];

				if (std::holds_alternative<operation_forget>(curr_nice_bags.op)) {
					operation_forget& op = std::get<operation_forget>(curr_nice_bags.op);
					if (op.vertex == endpoint_a) a_present = true;
					if (op.vertex == endpoint_b) b_present = true;
				}

				auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
				for (; itt != itt_end; ++itt) {
					if (*itt != parent) {  // Avoid revisiting the parent
						q.push({ *itt, current_vertex });
					}
				}
			}
			//initialize smallest bag
			auto [itt_curr, itt_curr_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
			for (;itt_curr < itt_curr_end; ++itt_curr) {
				//should be only one other bag besides its parent.
				if (*itt_curr != parent) {
					size_smallest_bag = nice_bags[*itt_curr].bag.size();
					index_smallest_bag = *itt_curr;
					parent_smallest_bag = current_vertex;
				}
			}
		}
		//keep track of the size and index of smallest bag you encountered & its parents index.

		//If a bag introduces one of our endpoints stop the traversal down that branch.
		std::queue<std::pair<int, int>> q; // {current_vertex, parent_vertex}

		q.push({ index_smallest_bag, parent_smallest_bag });

		while (!q.empty()) {
			auto [current_vertex, parent] = q.front();
			q.pop();

			nice_bag& curr_nice_bags = nice_bags[current_vertex];

			if (size_smallest_bag > curr_nice_bags.bag.size()) {
				size_smallest_bag = curr_nice_bags.bag.size();
				index_smallest_bag = current_vertex;
				parent_smallest_bag = parent;
			}

			//if one of the vertices is introduced, then you do not have to explore this branch any further.
			if (std::holds_alternative<operation_introduce>(curr_nice_bags.op)) {
				operation_introduce& op = std::get<operation_introduce>(curr_nice_bags.op);
				if (op.vertex == endpoint_a) continue;
				if (op.vertex == endpoint_b) continue;
			}

			auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_td);
			for (; itt != itt_end; ++itt) {
				if (*itt != parent) {
					q.push({ *itt, current_vertex });
				}
			}
		}
		//add vertex in between index smallest bag & parent.
		auto vertex = boost::add_vertex(graph_nice_td);
		if (parent_smallest_bag == -1) {
			boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, endpoint_a, endpoint_b, nice_bags[index_smallest_bag].bag));
			root_vertex = vertex;
		}
		else {
			//should never be the same index.
			if (index_smallest_bag < parent_smallest_bag) {
				boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
				boost::add_edge(parent_smallest_bag, vertex, graph_nice_td);
			}
			else {
				boost::add_edge(parent_smallest_bag, vertex, graph_nice_td);
				boost::add_edge(index_smallest_bag, vertex, graph_nice_td);
			}
			boost::remove_edge(index_smallest_bag, parent_smallest_bag, graph_nice_td);
			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE_EDGE, endpoint_a, endpoint_b, nice_bags[index_smallest_bag].bag));
		}
	}
}

void TREE_DECOMPOSITION::traverse_tree_decomposition(int parent_index, vertex v) {
	auto [itt, itt_end] = boost::adjacent_vertices(v, graph_td);
	int out_degree = boost::out_degree(v, graph_td);
	--out_degree; //don't count parent.

	//vertex is a leaf.
	if (out_degree == 0) {
		unfold_leaf_vertex(v);
		return;
	}

	//parent vertex is either a introduce or forget node.
	if (out_degree == 1) {
		for (;itt < itt_end; ++itt) {
			if (!(*itt == parent_index)) {
				
				unfold_parent_vertex(v, bags[v], *itt, bags[*itt]);
				traverse_tree_decomposition(v, *itt);
			}
		}
	}
	else if (out_degree == 2) {
		//vertex is a join vertex.
		for (;itt < itt_end; ++itt) {
			if (!(*itt == parent_index)) {
				//add vertex between join vertex and child vertex.
				boost::remove_edge(v, *itt, graph_nice_td);
				auto par_vertex = boost::add_vertex(graph_nice_td);
				//add in correct order.
				if (v > *itt) {
					boost::add_edge(*itt, par_vertex, graph_nice_td);
					boost::add_edge(v, par_vertex, graph_nice_td);
				}
				else {
					boost::add_edge(v, par_vertex, graph_nice_td);
					boost::add_edge(*itt, par_vertex, graph_nice_td);
				}
				nice_bags[v] = nice_bag(operation_enum::JOIN, bags[v]);

				//put in a empty placeholder.
				nice_bags.push_back(nice_bag());

				//will fill the empty placeholder.
				unfold_parent_vertex(par_vertex, bags[v], *itt, bags[*itt]);
				traverse_tree_decomposition(par_vertex, *itt);
			}
		}
	}
	else {
		throw std::invalid_argument("should not have more than 2 out degree");
	}
}

void TREE_DECOMPOSITION::unfold_parent_vertex(int parent, std::vector<int>& bag_parent, int child, std::vector<int>& bag_child) {
	int prev_vertex = child;

	std::vector<int> acc_bag = bags[child];

	// if bag_parent has child bag_child.
	// Elements which are in bag_child but not in bag_parent -> serie of forget vertices.
	// Elements which are in bag_parent but not in bag_child -> serie of introduce vertices.
	std::vector<int>forget_vertices = find_non_overlapping_vertices(bag_parent, bag_child);
	std::vector<int>introduce_vertices = find_non_overlapping_vertices(bag_child, bag_parent);

	if (introduce_vertices.size() > 1 || (introduce_vertices.size() > 0 && forget_vertices.size() > 0)) {
		//you will insert vertices so break up edge between parent and child.
		boost::remove_edge(parent, child, graph_nice_td);

		while (!forget_vertices.empty()) {
			//get element for forget vertices.
			int last_element = forget_vertices.back();
			forget_vertices.pop_back();

			//remove vertex from accumulated bag
			auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
			if (it != acc_bag.end()) {
				acc_bag.erase(it);
			}

			//create vertex and add edge.
			auto forget_vertex = boost::add_vertex(graph_nice_td);
			boost::add_edge(prev_vertex, forget_vertex, graph_nice_td);
			nice_bags.push_back(nice_bag(operation_enum::FORGET, last_element, acc_bag));
			prev_vertex = forget_vertex;
		}

		while (introduce_vertices.size() > 1) {
			//get element from introduce_vertices.
			int last_element = introduce_vertices.back();
			introduce_vertices.pop_back();

			//add element to accumulated bag.
			auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
			acc_bag.insert(it, last_element);

			//create vertex and add edge.
			auto introduce_vertex = boost::add_vertex(graph_nice_td);
			boost::add_edge(prev_vertex, introduce_vertex, graph_nice_td);
			nice_bags.push_back(nice_bag(operation_enum::INTRODUCE, last_element, acc_bag));
			prev_vertex = introduce_vertex;
		}

		//introduce_vertices.size() == 1.
		int last_element = introduce_vertices.back();
		
		//add edge.
		boost::add_edge(prev_vertex, parent, graph_nice_td);
		nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, bag_parent);
	}
	else if (introduce_vertices.size() == 1) {
		//you will insert vertices so break up edge between parent and child.
		int last_element = introduce_vertices.back();

		nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, bag_parent);
	}
	else if (forget_vertices.size() == 1) {
		//you will insert vertices so break up edge between parent and child.
		int last_element = forget_vertices.back();

		nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
	}
	else if (forget_vertices.size() > 1) {
		boost::remove_edge(parent, child, graph_nice_td);
		while (forget_vertices.size() > 1) {
			//get element from forget_vertices.
			int last_element = forget_vertices.back();
			forget_vertices.pop_back();

			//remove vertex from accumulated bag
			auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
			if (it != acc_bag.end()) {
				acc_bag.erase(it);
			}

			//create vertex and add edge.
			auto forget_vertex = boost::add_vertex(graph_nice_td);
			boost::add_edge(prev_vertex, forget_vertex, graph_nice_td);
			nice_bags.push_back(nice_bag(operation_enum::FORGET, last_element, acc_bag));
			prev_vertex = forget_vertex;
		}
		//forget_vertices == 1
		int last_element = forget_vertices.back();

		//add edge.
		boost::add_edge(prev_vertex, parent, graph_nice_td);
		nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
	}
	else {
		throw std::invalid_argument("something unforseen");
	}
}

//Get all elements which are in b, but not in a.
std::vector<int> find_non_overlapping_vertices(const std::vector<int>& a, const std::vector<int>& b) {
	std::vector<int> result;
	size_t i = 0, j = 0;

	while (j < b.size()) {
		if (i < a.size() && a[i] < b[j]) {
			//move pointer forward.
			i++;
		}
		else if (i < a.size() && a[i] == b[j]) {
			//element exists in both, move both pointers.
			i++;
			j++;
		}
		else {
			//Element is in b but not in a.
			result.push_back(b[j]);
			j++;
		}
	}
	return result;
}

void TREE_DECOMPOSITION::unfold_leaf_vertex(int vertex) {
	std::vector<int> bag = bags[vertex];

	//Create nice_bag on original place.
	int lastElement = bag.back();
	nice_bags[vertex] = nice_bag(operation_enum::INTRODUCE, lastElement, bag);
	bag.pop_back();

	int prev_vertex = vertex;
	while (!bag.empty()) {
		lastElement = bag.back();

		//Create new vertex.
		auto v = boost::add_vertex(graph_nice_td);
		nice_bags.push_back(nice_bag(operation_enum::INTRODUCE, lastElement, bag));
		auto e = boost::add_edge(prev_vertex, v, graph_nice_td);
		prev_vertex = v;
		bag.pop_back();
	}
	//create an empty leaf node.
	auto leaf_v = boost::add_vertex(graph_nice_td);
	//bag should be empty.
	nice_bags.push_back(nice_bag(operation_enum::LEAF, bag));
	auto leaf_e = boost::add_edge(prev_vertex, leaf_v, graph_nice_td);
}

void TREE_DECOMPOSITION::fill_instruction_stack() {
	depth_first_search(root_vertex, -1);
}

void TREE_DECOMPOSITION::depth_first_search(int start, int parent) {
	//depth first traversel. (the only splits should be join vertices).
	instruction_stack.push(&nice_bags[start]);
	auto [itt, itt_end] = boost::adjacent_vertices(start, graph_nice_td);
	for (; itt != itt_end; ++itt) {
		if (*itt != parent) {  // Avoid revisiting the parent
			depth_first_search(*itt, start);
		}
	}
}

void TREE_DECOMPOSITION::run_instruction_stack() {
	
	while (!instruction_stack.empty()) {
		//get top instruction from the stack. (is a pointer).
		nice_bag* instruction_ptr = instruction_stack.top();
		instruction_stack.pop();
		nice_bag& instruction = *instruction_ptr;

		//find which operation the instruction entails.

		//run if it is a leaf operation.
		if (std::holds_alternative<operation_leaf>(instruction.op)) {
			run_operation_leaf();
		}

		if (std::holds_alternative<operation_introduce>(instruction.op)) {
			//get object.
			operation_introduce correct_instruction = get<operation_introduce>(instruction.op);
			run_operation_introduce(instruction.bag, correct_instruction.vertex);
		}
	}
}

int find_index_in_bag(std::vector<int>& bag, int element) {
	auto it = std::lower_bound(bag.begin(), bag.end(), element); // point to the first element not less than the element.
	return it - bag.begin(); //returns the index.

}

int extract_bits(std::uint64_t encoding, int size_bag, int pos) {
	return (encoding >> (2 * (size_bag - 1 - pos))) & 0b11;
}

void TREE_DECOMPOSITION::run_operation_leaf() {
	//Push an empty partial solution onto the partial solution stack.
	std::unordered_map<std::uint64_t, int> empty_partial_solution;
	partial_solution_stack.push(empty_partial_solution);
}

void TREE_DECOMPOSITION::run_operation_forget() {

}

//(2-bit representation of colors of vertices). 0b01 -> black, 0b11 -> gray, 0b10 -> white.
const int COLORS[] = { 0b01, 0b11, 0b10 };
const int NUM_COLORS = 3;

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

void TREE_DECOMPOSITION::run_operation_introduce(std::vector<int>& bag, int introduced_vertex) {
	//find index of introduced vertex in the bag.
	int index_introduced_vertex = find_index_in_bag(bag, introduced_vertex);

	//create an empty partial solution.
	std::unordered_map<std::uint64_t, int> partial_solution;

	//get previous childs partial solution.
	std::unordered_map<std::uint64_t,int> child_partial_solution = partial_solution_stack.top();
	partial_solution_stack.pop();

	// create all possible coloring pairs for introduced operation.
	if (child_partial_solution.empty()) {
		std::vector<std::uint64_t> encoding_vector = generate_all_encoding(bag.size());
		for (uint64_t encoding : encoding_vector) {
			//gray.
			if (encoding == 3) {
				partial_solution.insert({ encoding, 0 });
				continue;
			}
			// white.
			if (encoding == 2) {
				partial_solution.insert({ encoding, 1 });
				continue;
			}
			// black.
			if (encoding == 1) {
				partial_solution.insert({ encoding, INT_MAX });
				continue;
			}
		}
		partial_solution_stack.push(partial_solution);
	}
	else {
		std::vector<std::pair<std::uint64_t, std::uint64_t>> encoding_vector = generate_all_encoding_introduce(bag.size(), introduced_vertex);

		for (const auto& [encoding, child_encoding] : encoding_vector) {
			int color = extract_bits(encoding, bag.size(), index_introduced_vertex);
			//gray.
			if (color == 3) {
				int solution = 0 + child_partial_solution[child_encoding];
				partial_solution.insert({ encoding, solution });
				continue;
			}
			//white.
			if (color == 2) {
				int solution = 1 + child_partial_solution[child_encoding];
				partial_solution.insert({ encoding, solution });
				continue;
			}
			//black.
			if (color == 1) {
				int solution = INT_MAX;
				partial_solution.insert({ encoding, solution });
				continue;
			}
		}
		partial_solution_stack.push(partial_solution);
	}
}

void TREE_DECOMPOSITION::run_operation_join() {

}

void TREE_DECOMPOSITION::run_operation_introduce_edge() {

}


operation::operation(operation_enum type) : opp(type) {};

operation_join::operation_join() : operation(operation_enum::JOIN) {};

operation_leaf::operation_leaf() : operation(operation_enum::LEAF) {};

operation_forget::operation_forget(int v) : operation(operation_enum::FORGET), vertex(v) {};

operation_introduce::operation_introduce(int v) : operation(operation_enum::INTRODUCE), vertex(v) {};

operation_introduce_edge::operation_introduce_edge(int v, int w) : operation(operation_enum::INTRODUCE_EDGE), endpoint_a(v), endpoint_b(w) {};

//placeholder.
nice_bag::nice_bag() {};



nice_bag::nice_bag(operation_enum operation, std::vector<int>bag_input) {
	bag = bag_input; 

	switch (operation) {
		case operation_enum::LEAF:
			op = operation_leaf();
			break;
		case operation_enum::JOIN:
			op = operation_join();
			break;
		default:
			throw std::invalid_argument("wrong operator (or wrong function constructor)");
	}
}

nice_bag::nice_bag(operation_enum operation, int v, std::vector<int>bag_input) {
	bag = bag_input;

	switch (operation) {
		case operation_enum::FORGET:
			op = operation_forget(v);
			break;
		case operation_enum::INTRODUCE:
			op = operation_introduce(v);
			break;
		default:
			throw std::invalid_argument("wrong operator (or wrong function constructor)");
	}
}

nice_bag::nice_bag(operation_enum operation, int v, int w, std::vector<int>bag_input) {
	bag = bag_input;

	switch (operation) {
		case operation_enum::INTRODUCE_EDGE:
			op = operation_introduce_edge(v, w);
			break;
		default:
			throw std::invalid_argument("wrong operator (or wrong function constructor)");

	}
}