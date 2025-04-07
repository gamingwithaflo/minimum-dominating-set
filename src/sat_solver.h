#pragma once
#include <boost/graph/adjacency_list.hpp>
#include "graph/context.h"
#include <stdint.h>
#include <stdlib.h>

#include "util/timer.h"
#include <algorithm>
#include <string>
#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"

typedef boost::adjacency_list<
    boost::vecS,                                  // Param:OutEdgeList (cointainer used for edge-list (vector))
    boost::vecS,                                  // Param:VertexList (cointainer used for vectexList (vector))
    boost::undirectedS,                           // Param:Directed
    boost::property<boost::vertex_index_t, int>,  // Param: Properties of the indecies
    boost::property<boost::edge_index_t, int>>    // Param: Properties of the indecies
    adjacencyListBoost;

typedef boost::graph_traits<adjacencyListBoost>::vertex_descriptor vertex;

void ds_sat_solver(MDS_CONTEXT& mds_context);