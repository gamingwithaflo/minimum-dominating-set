
#include "tree_decomposition.h"

TREE_DECOMPOSITION::TREE_DECOMPOSITION(std::vector<std::vector<int>> bags_input, adjacencyListBoost& g, int treewidth_input) {
	graph = g;
	bags = bags_input;
	treewidth = treewidth_input;
}