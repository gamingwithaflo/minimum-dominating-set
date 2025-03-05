
#include "tree_decomposition.h"

TREE_DECOMPOSITION::TREE_DECOMPOSITION(std::vector<std::vector<int>> bags_input, adjacencyListBoost& g, int treewidth_input) {
	graph = g;
	bags = bags_input;
	treewidth = treewidth_input;
}

void TREE_DECOMPOSITION::create_nice_tree_decomposition() {

}

operation::operation(operation_enum type) : opp(type) {};

operation_join::operation_join() : operation(operation_enum::JOIN) {};

operation_leaf::operation_leaf() : operation(operation_enum::LEAF) {};

operation_forget::operation_forget(int v) : operation(operation_enum::FORGET), vertex(v) {};

operation_introduce::operation_introduce(int v) : operation(operation_enum::INTRODUCE), vertex(v) {};

operation_introduce_edge::operation_introduce_edge(int v, int w) : operation(operation_enum::INTRODUCE_EDGE), endpoint_a(v), endpoint_b(w) {};