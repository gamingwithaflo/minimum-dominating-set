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

NICE_TREE_DECOMPOSITION::NICE_TREE_DECOMPOSITION(std::vector<std::vector<int>> bags, adjacencyListBoost g)
{
    //;
}

std::vector<int> which_edges_must_be_introduced(const adjacencyListBoost& original_graph, std::vector<uint>& bag_child, int forget_vertex) {
	std::vector<int>must_introduce;
    must_introduce.reserve(bag_child.size());
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
}

//helper function.
int NICE_TREE_DECOMPOSITION::introduce_edge_smart(int source, int target, int parent, int child, adjacencyListBoost& original_graph){
    int smallest_bag_index = INT_MAX;
    int smallest_bag_size = INT_MAX;
    int smallest_bag_parent = INT_MAX;

    std::queue<std::pair<int,int>> queue; //child, parent.

    queue.emplace(child, parent);
    while (!queue.empty())
    {
        auto [curr_child, curr_parent] = queue.front();
        queue.pop();

        //If one of the two end_points get introduced we know it can't be further in the operation.
        if (std::holds_alternative<operation_introduce>(nice_bags[curr_parent].op)) {
            auto & op = std::get<operation_introduce>(nice_bags[curr_parent].op);
            if (op.vertex == source || op.vertex == target) continue;
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
    //vertex gets forgotten & introduced after each other.
    if (smallest_bag_parent == INT_MAX){
        //put vertex between parent & child.
        boost::remove_edge(parent, child, graph_nice_tree_decomposition);
        boost::add_edge(parent, nice_bags.size(), graph_nice_tree_decomposition);
        boost::add_edge(nice_bags.size(), child, graph_nice_tree_decomposition);
        nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[child].bag);
        return (nice_bags.size() -1);
    }
    //General case.
    boost::remove_edge(smallest_bag_parent, smallest_bag_index, graph_nice_tree_decomposition);
    boost::add_edge(smallest_bag_parent, nice_bags.size(), graph_nice_tree_decomposition);
    boost::add_edge(nice_bags.size(),smallest_bag_index , graph_nice_tree_decomposition);
    nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[smallest_bag_index].bag);
    return (smallest_bag_parent == parent) ? nice_bags.size() - 1 : parent;
}

//Needs to be refactored.
void NICE_TREE_DECOMPOSITION::introduce_all_edges(const adjacencyListBoost& original_graph) {

    //find all possible edge combinations possible from root_node bag.
    std::vector<std::pair<int,int>> pairs = find_all_pairs(nice_bags[root_vertex].bag);

    for (auto& [source, target] : pairs) {
        //If the edge exists in the original graph. Introduce it above and make it the new source.
        if (auto [edge, exists] = boost::edge(source, target, original_graph); exists) {
            boost::add_edge(nice_bags.size(), root_vertex, graph_nice_tree_decomposition);
            uint original_root_vertex = root_vertex;
            root_vertex = nice_bags.size();
            nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, source, target, nice_bags[original_root_vertex].bag);
        }

        //root vertex is adjacent to only one vertex.
        auto [itt, itt_end] = boost::adjacent_vertices(root_vertex, graph_nice_tree_decomposition);

        std::queue<std::pair<int, int>> q;

        q.emplace(*itt, root_vertex); // {child, parent}

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
                    boost::remove_edge(parent, current_vertex, graph_nice_tree_decomposition);
                    //vertex introduce_edge = boost::add_vertex(graph_nice_tree_decomposition);
                    //keep sorted.
                    if (parent > current_vertex) {
                        boost::add_edge(nice_bags.size(),current_vertex, graph_nice_tree_decomposition);
                        boost::add_edge(parent, nice_bags.size(), graph_nice_tree_decomposition);
                    }
                    else {
                        boost::add_edge(parent, nice_bags.size(), graph_nice_tree_decomposition);
                        boost::add_edge(nice_bags.size(),current_vertex, graph_nice_tree_decomposition);
                    }
                    parent = nice_bags.size();
                    nice_bags.emplace_back(operation_enum::INTRODUCE_EDGE, end_point, forget_vertex, nice_bags[current_vertex].bag);
                }
            }
            //Traverse the whole nice tree decomposition.
            for (auto [itt, itt_end] = boost::adjacent_vertices(current_vertex, graph_nice_tree_decomposition); itt != itt_end; ++itt) {
                if (*itt != parent) {  // Avoid revisiting the parent
                    q.emplace(*itt, current_vertex);
                }
            }
        }
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


// void NICE_TREE_DECOMPOSITION::unfold_leaf_vertex(int vertex) {
//     std::vector<int> bag = bags[vertex];
//
//     //Create nice_bag on original_place.
//     int lastElement = bag.back();
//     nice_bags[vertex] = nice_bag(operation_enum::INTRODUCE, lastElement, bag);
//     bag.pop_back();
//
//     int prev_vertex = vertex;
//     //create a chain of introduces.
//     while (!bag.empty()) {
//         lastElement = bag.back();
//
//         //Create new vertex.
//         auto v = boost::add_vertex(graph_nice_tree_decomposition);
//         nice_bags.emplace_back(operation_enum::INTRODUCE, lastElement, bag);
//         auto e = boost::add_edge(prev_vertex, v, graph_nice_tree_decomposition);
//         prev_vertex = v;
//         bag.pop_back();
//     }
//     //create an empty leaf node.
//     auto leaf_v = boost::add_vertex(graph_nice_tree_decomposition);
//     //bag should be empty.
//     nice_bags.emplace_back(operation_enum::LEAF, bag);
//     auto leaf_e = boost::add_edge(prev_vertex, leaf_v, graph_nice_tree_decomposition);
// }
//
// //Get all elements which are in b, but not in a.
// std::vector<int> find_non_overlapping_vertices(const std::vector<int>& a, const std::vector<int>& b) {
//     std::vector<int> result;
//     result.reserve(a.size() + b.size()); //in the worst case, none of the vertices overlap.
//     size_t i = 0, j = 0;
//
//     while (j < b.size()) {
//         if (i < a.size() && a[i] < b[j]) {
//             //move pointer forward.
//             i++;
//         }
//         else if (i < a.size() && a[i] == b[j]) {
//             //element exists in both, move both pointers.
//             i++;
//             j++;
//         }
//         else {
//             //Element is in b but not in a.
//             result.push_back(b[j]);
//             j++;
//         }
//     }
//     return result;
// }
//
// // Helper function to process "introduce" vertices
// void process_introduce_vertices(
//     std::vector<int>& introduce_vertices,
//     std::vector<int>& acc_bag,
//     int& prev_vertex,
//     adjacencyListBoost& graph_nice_td,
//     std::vector<nice_bag>& nice_bags)
// {
//     while (introduce_vertices.size() > 1) {
//         int last_element = introduce_vertices.back();
//         introduce_vertices.pop_back();
//
//         // Add element to accumulated bag
//         auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element);
//         acc_bag.insert(it, last_element);
//
//         // Create vertex and add edge
//         auto introduce_vertex = boost::add_vertex(graph_nice_td);
//         boost::add_edge(prev_vertex, introduce_vertex, graph_nice_td);
//         nice_bags.emplace_back(operation_enum::INTRODUCE, last_element, acc_bag);
//         prev_vertex = introduce_vertex;
//     }
// }
//
// //Helper function to create the nice forget bag.
// void process_forget_vertices(
//     std::vector<int>& forget_vertices,
//     std::vector<int>& acc_bag,
//     int& prev_vertex,
//     adjacencyListBoost& graph_nice_td,
//     std::vector<nice_bag>& nice_bags)
// {
//     while (!forget_vertices.empty()) {
//         int last_element = forget_vertices.back();
//         forget_vertices.pop_back();
//
//         // Remove vertex from accumulated bag if it exists.
//         if (auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element); it != acc_bag.end()) {
//             acc_bag.erase(it);
//         }
//
//         // Create vertex and add edge
//         auto forget_vertex = boost::add_vertex(graph_nice_td);
//         boost::add_edge(prev_vertex, forget_vertex, graph_nice_td);
//         nice_bags.emplace_back(operation_enum::FORGET, last_element, acc_bag);
//         prev_vertex = forget_vertex;
//     }
// }
//
// void NICE_TREE_DECOMPOSITION::unfold_parent_vertex(int parent, int child, const std::vector<int>& bag_parent, const std::vector<int>& bag_child) {
//     int prev_vertex = child;
//     std::vector<int> acc_bag = bags[child];
//
//     // vertices which are in bag_child but not in bag_parent (need to be forgotten).
//     std::vector<int> forget_vertices = find_non_overlapping_vertices(bag_parent, bag_child);
//     // Elements
//     std::vector<int> introduce_vertices = find_non_overlapping_vertices(bag_child, bag_parent);
//
//     //We need to create at least 1 bag, between parent and child.
//     if (introduce_vertices.size() > 1 || (!introduce_vertices.empty() && !forget_vertices.empty())) {
//         boost::remove_edge(parent, child, graph_nice_tree_decomposition);
//
//         process_forget_vertices(forget_vertices, acc_bag, prev_vertex, graph_nice_tree_decomposition, nice_bags);
//
//         process_introduce_vertices(introduce_vertices, acc_bag, prev_vertex, graph_nice_tree_decomposition, nice_bags);
//
//         //Final introduction of the last element (which is the original parent).
//         const int last_element = introduce_vertices.back();
//         boost::add_edge(prev_vertex, last_element, graph_nice_tree_decomposition);
//         nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, acc_bag);
//     }
//     else if (introduce_vertices.size() == 1) {
//         const int last_element = forget_vertices.back();
//         nice_bags[parent] = nice_bag(operation_enum::INTRODUCE, last_element, bag_parent);
//     }
//     //given: Introduce_vertices is empty.
//     else if (forget_vertices.size() == 1) {
//         //If only one element which is a Forget.
//         const int last_element = introduce_vertices.back();
//         nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
//     }
//     else
//     {
//         boost::remove_edge(parent, child, graph_nice_tree_decomposition);
//
//         // Process "forget" vertices
//         while (forget_vertices.size() > 1) {
//             //get element from forget_vertices.
//             int last_element = forget_vertices.back();
//             forget_vertices.pop_back();
//
//             //remove vertex from accumulated bag
//             if (auto it = std::lower_bound(acc_bag.begin(), acc_bag.end(), last_element); it != acc_bag.end()) {
//                 acc_bag.erase(it);
//             }
//
//             //create vertex and add edge.
//             auto forget_vertex = boost::add_vertex(graph_nice_tree_decomposition);
//             boost::add_edge(prev_vertex, forget_vertex, graph_nice_tree_decomposition);
//             nice_bags.emplace_back(operation_enum::FORGET, last_element, acc_bag);
//             prev_vertex = forget_vertex;
//         }
//
//         // Final forget of the last element
//         const int last_element = forget_vertices.back();
//         boost::add_edge(prev_vertex, parent, graph_nice_tree_decomposition);
//         nice_bags[parent] = nice_bag(operation_enum::FORGET, last_element, bag_parent);
//     }
// }

// void NICE_TREE_DECOMPOSITION::traverse_tree_decomposition(const int parent_index, const vertex v) {
//     //we walk through the original graph, so changes to nice_graph_td will not efect your path.
// 	auto [itt, itt_end] = boost::adjacent_vertices(v, graph_tree_decomposition);
// 	int out_degree = boost::out_degree(v, graph_tree_decomposition);
// 	--out_degree; //don't count parent.
//
// 	//vertex is a leaf.
// 	if (out_degree == 0) {
// 		unfold_leaf_vertex(v);
// 		return;
// 	}
//
// 	//parent vertex is either a introduce or forget node.
// 	if (out_degree == 1) {
// 		for (;itt < itt_end; ++itt) {
// 			if (!(*itt == parent_index)) {
//
// 				unfold_parent_vertex(v, *itt ,bags[v], bags[*itt]);
// 				traverse_tree_decomposition(v, *itt);
// 			}
// 		}
// 	}
// 	else {
// 		nice_bags[v] = nice_bag(operation_enum::JOIN, bags[v]);
// 		std::queue<int>children;
// 		for (;itt < itt_end; ++itt) {
// 			//push all children vertices on the queue.
// 			if (!(*itt == parent_index)) {
// 				boost::remove_edge(v, *itt, graph_tree_decomposition);
//
// 				vertex child = boost::add_vertex(graph_tree_decomposition);
//
// 				//add all edges to parent (v) and if it is not adjacent to v remove it.
// 				boost::add_edge(v, child, graph_tree_decomposition);
//
// 				boost::add_edge(*itt, child, graph_tree_decomposition);
// 				children.push(child);
//
// 				//put in a empty placeholder.
// 				nice_bags.emplace_back();
//
// 				//will fill the empty placeholder.
// 				unfold_parent_vertex(child, *itt, bags[v] ,bags[*itt]);
// 				traverse_tree_decomposition(v, *itt); // we need v, as that is in the original graph the parent.
// 			}
// 		}
// 	    // Tree decompositions can have for than 2 children so in a nice tree decomposition we need to balance it out.
// 		// if queue is not empty. (child_a & child_b are not adjacent to v)
// 		while (children.size() != 2) { // if it equals two, it already has been connected correctly.
// 			int child_a = children.front();
// 			children.pop();
// 			int child_b = children.front();
// 			children.pop();
//
// 			boost::remove_edge(v, child_a, graph_tree_decomposition);
// 			boost::remove_edge(v, child_b, graph_tree_decomposition);
//
// 			vertex new_child = boost::add_vertex(graph_tree_decomposition);
// 			boost::add_edge(v, new_child, graph_tree_decomposition);
// 			//add edges, ensuring that the order is maintained.
// 			if (child_a > child_b) {
// 				boost::add_edge(child_b, new_child, graph_tree_decomposition);
// 				boost::add_edge(child_a, new_child, graph_tree_decomposition);
// 			}
// 			else {
// 				boost::add_edge(child_a, new_child, graph_tree_decomposition);
// 				boost::add_edge(child_b, new_child, graph_tree_decomposition);
// 			}
// 			children.push(new_child);
// 			nice_bags.emplace_back(operation_enum::JOIN, bags[v]);
// 		}
// 	}
// }

