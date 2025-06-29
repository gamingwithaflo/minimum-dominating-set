//
// Created by floris on 3/17/25.
//
#include "nice_tree_decomposition.h"

#include <queue>
#include <htd/AdaptiveWidthMinimizingTreeDecompositionAlgorithm.hpp>
#include <utility>

NICE_TREE_DECOMPOSITION::NICE_TREE_DECOMPOSITION(adjacencyListBoost& reduced_graph, const htd::ITreeDecomposition* decomposition) {
    //decomposition in 1 indexed, and my structure is 0 indexed.
    const int edge_cnt = boost::num_edges(reduced_graph);
    const int vertex_cnt = decomposition->vertexCount();

    //initialize graph.
    treewidth = decomposition->maximumBagSize() - 1;
    nice_bags.reserve(vertex_cnt + edge_cnt);
    graph_nice_tree_decomposition = adjacencyListBoostDirected(vertex_cnt + edge_cnt);

    root_vertex = 0;

    std::queue<std::pair<htd::vertex_t, int>>children; // child, parent.

    children.emplace(decomposition->root(), -1);

    //Breath first traversal
    while (!children.empty()){
        const auto  [current_vertex, parent_index] = children.front();
        children.pop();

        auto find_children = decomposition->children(current_vertex);
        auto const& bag_content = decomposition->bagContent(current_vertex);
        //HTD are 1-indexed, my program is 0 indexed.
        std::vector<uint> new_bag_content(bag_content.size());
        std::transform(bag_content.begin(), bag_content.end(), new_bag_content.begin(), [](uint content) {
            return content - 1;
        });

        if (decomposition->isIntroduceNode(current_vertex)){
            auto introduced_vertices = decomposition->introducedVertices(current_vertex);

            //Check if not to many vertices get introduced at once.
            if (introduced_vertices.size() != 1 || find_children.size() != 1)
            {
                throw std::runtime_error("Either incorrect number of children, or incorrect number introduced vertices.");
            }
            for (auto child : find_children) {
                    children.emplace(child, nice_bags.size());
            }
            if (!decomposition->isRoot(current_vertex)){
                //If it is not THE root node, add edge.
                boost::add_edge(parent_index, nice_bags.size(), graph_nice_tree_decomposition);
            }
            nice_bags.emplace_back(operation_enum::INTRODUCE, introduced_vertices[0]-1, new_bag_content);
            continue;
        }

        if (decomposition->isLeaf(current_vertex)){
            if (decomposition->bagSize(current_vertex) != 0 && find_children.size() != 0) {
                throw std::runtime_error("Leaf bags should be empty");
            }
            if (!decomposition->isRoot(current_vertex)){
                boost::add_edge(parent_index, nice_bags.size(), graph_nice_tree_decomposition);

            }
            nice_bags.emplace_back(operation_enum::LEAF, std::vector<uint>());
            continue;
        }

        if (decomposition->isForgetNode(current_vertex)){
            auto forget_vertices = decomposition->forgottenVertices(current_vertex);

            //Check if not to many vertices get introduced at once.
            if (forget_vertices.size() != 1 || find_children.size() != 1)
            {
                throw std::runtime_error("Either incorrect number of children, or incorrect number of forget vertices.");
            }
            for (auto child : find_children) {
                children.emplace(child, nice_bags.size());
            }

            if (!decomposition->isRoot(current_vertex))
            {
                //If it is not THE root node, add edge.
                boost::add_edge(parent_index, nice_bags.size(), graph_nice_tree_decomposition);
            }
            nice_bags.emplace_back(operation_enum::FORGET, forget_vertices[0]-1, new_bag_content);
            continue;
        }

        if (decomposition->isJoinNode(current_vertex)){
            //Check if not to many vertices get introduced at once.
            if (find_children.size() != 2)
            {
                throw std::runtime_error("incorrect number of children");
            }
            for (auto child : find_children) {
                children.emplace(child, nice_bags.size());
            }

            if (!decomposition->isRoot(current_vertex)){
                //If it is not THE root node, add edge.
                boost::add_edge(parent_index ,nice_bags.size(), graph_nice_tree_decomposition);
            }

            nice_bags.emplace_back(operation_enum::JOIN, new_bag_content);
        }
    }
    introduce_all_edges_smart(reduced_graph);
}

//helper function.
std::vector<std::pair<int, int>> find_all_pairs(const std::vector<uint>& bag) {
    std::vector<std::pair<int, int>> res;
    if (bag.size() < 2) {
        //not enough elements to return pairs.
        return res;
    }

    const int max_pairs = bag.size() * (bag.size() - 1) / 2;
    res.reserve(max_pairs);

    for (size_t i = 0; i < bag.size(); ++i) {
        for (size_t j = i + 1; j < bag.size(); ++j) { // Avoid duplicate pairs
            res.emplace_back(bag[i], bag[j]);
        }
    }
    return res;
}

std::vector<int> which_edges_must_be_introduced(const adjacencyListBoost& original_graph, std::vector<uint>& bag_child, int forget_vertex) {
	std::vector<int>must_introduce;
	for (int end_point : bag_child) {
		if (end_point != forget_vertex) {
			auto [edge, exists] = boost::edge(forget_vertex, end_point, original_graph);
			if (exists) {
				must_introduce.push_back(end_point);
			}
		}
	}
	return must_introduce;
}

//we remove the edges from the original graph which are already introduced.
void NICE_TREE_DECOMPOSITION::introduce_all_edges_smart(adjacencyListBoost& original_graph)
{
    //All vertices in the root_bag, will never be forgotten. So either need to be introduced above the root node or the smallest bag both enpoints are present in.
    std::vector<std::pair<int,int>> pairs = find_all_pairs(nice_bags[root_vertex].bag);
    for (auto& [source, target] : pairs) {
        if (auto [edge, exists] = boost::edge(source, target, original_graph); exists) {
            //for each edge which actually exists, take a walker.
            int _ = introduce_edge_smart(source, target, -1, root_vertex,original_graph);
        }
    }
    //Introduce all other vertices.
    std::queue<std::pair<int,int>> q;

    auto [itt, itt_end] = boost::out_edges(root_vertex, graph_nice_tree_decomposition); // should be 1.
    for (; itt != itt_end; ++itt) {
        //put all childs in the breath first traversal.
        int new_child = boost::target(*itt, graph_nice_tree_decomposition);
        q.emplace(new_child, root_vertex); // child, parent
    }

    while (!q.empty()) {
        auto [curr_child, curr_parent] = q.front();
        q.pop();

        //If the parent vertex is a forget node, all edges which share an endpoint with the forgotten node, need to be introduced before this.
        if (std::holds_alternative<operation_forget>(nice_bags[curr_parent].op)){
            auto & op = std::get<operation_forget>(nice_bags[curr_parent].op);
            std::vector<int> introduce = which_edges_must_be_introduced(original_graph,nice_bags[curr_child].bag, op.vertex);

            for (auto into : introduce) {
                curr_parent = introduce_edge_smart(op.vertex, into, curr_parent, curr_child, original_graph);
            }
        }
        //fill queue with children of current node.
        for (auto [itt, itt_end] = boost::out_edges(curr_child, graph_nice_tree_decomposition); itt != itt_end; ++itt) {
            int new_child = boost::target(*itt, graph_nice_tree_decomposition);
            q.emplace(new_child, curr_child);
        }
    }
    if (boost::num_edges(original_graph) != 0) {
        throw std::runtime_error("Not every edge has been introduced.");
    }
}

//helper function.
int NICE_TREE_DECOMPOSITION::introduce_edge_smart(int source, int target, int parent, int child, adjacencyListBoost& original_graph){
    int smallest_bag_index = child;
    int smallest_bag_size = nice_bags[child].bag.size();
    int smallest_bag_parent = parent;

    std::queue<std::pair<int,int>> queue; //child, parent.

    queue.emplace(child, parent);
    while (!queue.empty())
    {
        auto [curr_child, curr_parent] = queue.front();
        queue.pop();

        if (curr_parent != -1)
        {
            //If one of the two end_points get introduced we know it can't be further in the operation.
            if (std::holds_alternative<operation_introduce>(nice_bags[curr_parent].op)) {
                auto & op = std::get<operation_introduce>(nice_bags[curr_parent].op);
                if (op.vertex == source || op.vertex == target) continue;
            }
        }
        //Update if you find a better solution.
        if (const auto& child_bag_size = nice_bags[curr_child].bag.size(); child_bag_size < smallest_bag_size) {
            smallest_bag_size = child_bag_size;
            smallest_bag_index = curr_child;
            smallest_bag_parent = curr_parent;
        }

        for (auto [itt, itt_end] = boost::out_edges(curr_child, graph_nice_tree_decomposition); itt != itt_end; ++itt) {
            int new_child = boost::target(*itt, graph_nice_tree_decomposition);
            queue.emplace(new_child, curr_child);
        }
    }
    //keep track of already introduced vertices.
    boost::remove_edge(source,target, original_graph);

    //Smallest bag is the current_root node.
    if (smallest_bag_parent == -1) {
        //smallest bag is the root.
        boost::add_edge(nice_bags.size(),root_vertex,graph_nice_tree_decomposition);
        int original_root_vertex = root_vertex;
        root_vertex = nice_bags.size();
        nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[original_root_vertex].bag);
        return (nice_bags.size() - 1);
        //vertex gets forgotten & introduced after each other.
    }
    //General case.
    boost::remove_edge(smallest_bag_parent, smallest_bag_index, graph_nice_tree_decomposition);
    boost::add_edge(smallest_bag_parent, nice_bags.size(), graph_nice_tree_decomposition);
    boost::add_edge(nice_bags.size(),smallest_bag_index , graph_nice_tree_decomposition);
    nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[smallest_bag_index].bag);
    return (smallest_bag_parent == parent) ? nice_bags.size() - 1 : parent;
}

//Needs to be refactored.
void NICE_TREE_DECOMPOSITION::introduce_all_edges(adjacencyListBoost& original_graph) {

    //find all possible edge combinations possible from root_node bag.
    std::vector<std::pair<int,int>> pairs = find_all_pairs(nice_bags[root_vertex].bag);

    for (auto& [source, target] : pairs)
    {
        //If the edge exists in the original graph. Introduce it above and make it the new source.
        if (auto [edge, exists] = boost::edge(source, target, original_graph); exists) {
            boost::remove_edge(source, target, original_graph);
            boost::add_edge(nice_bags.size(), root_vertex, graph_nice_tree_decomposition);
            uint original_root_vertex = root_vertex;
            root_vertex = nice_bags.size();
            nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[original_root_vertex].bag);
        }
    }

    std::queue<std::pair<int, int>> q;

    //root vertex is adjacent to only one vertex.
    auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_nice_tree_decomposition);
    for (; itt != itt_end; ++itt) {
        q.emplace(*itt, root_vertex); // {child, parent}
    }

    //find the highest bag (from root) where both source and target are present.
    while (!q.empty()) {
        auto [current_vertex, parent] = q.front();
        q.pop();

        std::vector<uint>& bag_child = nice_bags[current_vertex].bag;

        if (nice_bag& bag_parent = nice_bags[parent]; std::holds_alternative<operation_forget>(bag_parent.op)) {
            const auto& op = std::get<operation_forget>(bag_parent.op);
            int forget_vertex = op.vertex;
            //A vertex is about to be forgotten, so all possible edges need to be introduced beforehand.
            std::vector<int> must_introduce = which_edges_must_be_introduced(original_graph, bag_child, forget_vertex);

            for (int end_point : must_introduce) {
                boost::remove_edge(end_point, forget_vertex, original_graph);
                boost::remove_edge(parent, current_vertex, graph_nice_tree_decomposition);

                boost::add_edge(nice_bags.size(),current_vertex, graph_nice_tree_decomposition);
                boost::add_edge(parent, nice_bags.size(), graph_nice_tree_decomposition);
                parent = nice_bags.size();
                nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, end_point, forget_vertex, bag_child);
                }
            }
            //Traverse the whole nice tree decomposition.
            for (auto [itt, itt_end] = boost::out_edges(current_vertex, graph_nice_tree_decomposition); itt != itt_end; ++itt) {
                int target = boost::target(*itt, graph_nice_tree_decomposition);
                q.emplace(target, current_vertex);
            }
        }
    if (boost::num_edges(original_graph) != 0) {
        throw std::runtime_error("Not every edge has been introduced.");
    }
}

operation_join::operation_join() = default;

operation_leaf::operation_leaf() = default;

operation_forget::operation_forget(const int v) : vertex(v) {};

operation_introduce::operation_introduce(const int v) : vertex(v) {};

operation_introduce_edge::operation_introduce_edge(const int source, const int target) : source(source), target(target) {};

//constructor for Join and Leaf.
nice_bag::nice_bag(operation_enum operation, std::vector<uint>bag_input) {
    bag = std::move(bag_input);

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


//constructor for Forget and Introduce vertex.
nice_bag::nice_bag(operation_enum operation, int v, std::vector<uint>bag_input) {
    bag = std::move(bag_input);

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

//constructor for Introduce edge.
nice_bag::nice_bag(operation_enum operation, int v, int w, std::vector<uint>bag_input)
{
    bag = std::move(bag_input);

    if (operation == operation_enum::INTRODUCE_EDGE)
    {
        op = operation_introduce_edge(v, w);
    } else {
        throw std::invalid_argument("wrong operation (or wrong function constructor");
    }
}

