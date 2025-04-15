#pragma once
#include "graph/context.h"

#include "util/logger.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace reduce {
    void reduction_rule_manager(MDS_CONTEXT& mds_context, strategy_reduction strategy);

    bool reduction_l_rule(MDS_CONTEXT& mds_context, std::vector<int>& l_vertices);

    std::vector<vertex> bfs_get_distance_three(MDS_CONTEXT& mds_context, vertex v);

    void reduce_alber(MDS_CONTEXT& mds_context, bool run_rule_2);

    void reduce_ijcai(MDS_CONTEXT& mds_context, bool run_rule_2);

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

    bool is_superset(MDS_CONTEXT& mds_context, std::unordered_set<int>& subset_w, std::vector<int>& w_alter);
}