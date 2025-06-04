#pragma once
#include "graph/context.h"

#include "util/logger.h"

#include <chrono>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace reduce {
    void reduction_rule_manager(MDS_CONTEXT& mds_context, strategy_reduction& strategy, int l, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration);

    bool reduction_l_rule(MDS_CONTEXT& mds_context, std::vector<int>& l_vertices);

    void generateSelectors( const std::vector<int>& W_sizes,std::vector<int>& current, int index, std::vector<std::vector<int>>& selectorVertices);

    void reduce_l_alber_dense(MDS_CONTEXT& mds_context, int l, std::atomic<bool>& stop_flag);

    void execute_l_alber_two(MDS_CONTEXT& mds_context, int l, int vertex);

    std::vector<vertex> bfs_get_distance_three_generalized_original_graph(MDS_CONTEXT& mds_context, adjacencyListBoost& original_graph, std::vector<int>& vertices);

    void reduce_l_alber(MDS_CONTEXT& mds_context, int l, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration);

    bool execute_l_alber(MDS_CONTEXT& mds_context, int l, int counter, std::vector<int> vertices);

    std::vector<vertex> bfs_get_distance_three_generalized(MDS_CONTEXT& mds_context, std::vector<int>& vertices);

    std::vector<vertex> bfs_get_distance_three(MDS_CONTEXT& mds_context, vertex v);

    void reduce_alber(MDS_CONTEXT& mds_context, bool run_rule_2);

    void reduce_ijcai(MDS_CONTEXT& mds_context, bool run_rule_2, bool theory_strategy, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::seconds timeout_duration);

    bool reduce_neighborhood_single_vertex(MDS_CONTEXT& mds_context, vertex u);

    bool reduce_neighborhood_pair_vertices(MDS_CONTEXT& mds_context, vertex v, vertex w);

    bool reduce_neighborhood_pair_vertices_ijcai(MDS_CONTEXT& mds_context, vertex v, vertex w);

    std::unordered_set<vertex> get_distance_three(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_one(MDS_CONTEXT& mds_context, vertex v);

    bool simple_rule_five(MDS_CONTEXT& mds_context, vertex v);

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