//
// Created by floris on 3/18/25.
//

#include <boost/graph/adjacency_list.hpp>
#include "nice_tree_decomposition.h"

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (container used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (container used for vertex List (vector))
    boost::undirectedS,                           // Param: undirected.
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indices
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indices
    adjacencyListBoost;

std::unique_ptr<NICE_TREE_DECOMPOSITION> generate_td(adjacencyListBoost& reduced_graph);
