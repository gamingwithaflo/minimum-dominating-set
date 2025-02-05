#pragma once

#include <fstream>
#include <sstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

typedef boost::adjacency_list<                    
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;

namespace parse {
    adjacencyListBoost read_pace_2024(std::istream& is);

    adjacencyListBoost load_pace_2024(char const* path);
} 
