#pragma once
#include <boost/graph/adjacency_list.hpp>
#include "graph/context.h"

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

std::vector<int> sat_solver_dominating_set(MDS_CONTEXT& mds_context, adjacencyListBoost& graph, std::unordered_map<int,int>& newToOldIndex);