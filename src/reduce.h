#pragma once
#include "graph/context.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace reduce {
    void reduce_graph(MDS_CONTEXT& mds_context);

    void log_reduce_graph(MDS_CONTEXT& mds_context);

    bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u);

    bool reduce_neighborhood_pair_vertices(MDS_CONTEXT& mds_context, vertex v, vertex w);

    bool simple_rule_one(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_two(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_three(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_four(MDS_CONTEXT& mds_context, vertex v);

    bool dominated_subset_rule(MDS_CONTEXT& mds_context, vertex v);
}