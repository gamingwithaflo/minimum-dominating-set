#pragma once
#include "graph/graph_io.h"
#include "graph/context.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace reduce {
    void reduce_graph(MDS_CONTEXT& mds_context);

    void reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u);
}