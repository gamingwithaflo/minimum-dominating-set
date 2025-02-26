#pragma once
#include "graph/context.h"

#include "util/logger.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace reduce {
    void refractored_reduce_graph(MDS_CONTEXT& mds_context);

    void reduce_ijcai(MDS_CONTEXT& mds_context);

    bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u);

    bool reduce_neighborhood_pair_vertices(MDS_CONTEXT& mds_context, vertex v, vertex w);

    bool reduce_neighborhood_pair_vertices_ijcai(MDS_CONTEXT& mds_context, vertex v, vertex w);

    std::unordered_set<vertex> get_distance_three(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_one(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_two(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_three(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_four(MDS_CONTEXT& mds_context, vertex v);

    bool contains(MDS_CONTEXT& mds_context, vertex v, vertex u);

    bool check_subset(MDS_CONTEXT& mds_context, vertex v);

    bool reduce_subset(MDS_CONTEXT& mds_context, vertex v);

    bool reduce_ignore(MDS_CONTEXT& mds_context, vertex v);

    bool reduce_single_dominator(MDS_CONTEXT& mds_context, vertex v);
}