#pragma once
#include <string>
#include <vector>

void initialize_logger();

class Logger {
public:
	static bool flag_sr_1;
	static bool flag_sr_2;
	static bool flag_sr_3;
	static bool flag_sr_4;
	static bool flag_neigh_single;
	static bool flag_neigh_pair;

	static int cnt_simple_rule_one;
	static int cnt_simple_rule_two;
	static int cnt_simple_rule_three_dot_one;
	static int cnt_simple_rule_three_dot_two;
	static int cnt_simple_rule_four;
	static int cnt_reduce_neighborhood_single_vertex; //reduce a vertex.
	static int cnt_reduce_neighborhood_single_vertex_guard;
	static int cnt_reduce_neighborhood_pair_vertex_single; //can be dominated by only v or only w
	static int cnt_reduce_neighborhood_pair_vertex_both; //dont know whether to choose v or w
	static int cnt_reduce_neighborhood_pair_vertex_either; //has to be dominated by both v and w.

	static bool no_undetermined_vertices;
	static std::vector<int> solution_vector_with_reduction;
	static std::vector<int> solution_vector_without_reduction;

	static int att_simple_rule_one;
	static int att_simple_rule_two;
	static int att_simple_rule_three;
	static int att_simple_rule_four;
	static int att_reduce_neighborhood_single_vertex;
	static int att_reduce_neighborhood_pair_vertex;

	static long long execution_ilp_without_reduction;
	static long long execution_ilp_with_reduction;
	static long long execution_reduction;

	static bool is_planar;
};

void output_loginfo(std::string& path, std::vector<int>& included, std::vector<int>& dominated, std::vector<int>& removed, std::vector<int>& ignored, std::vector<int>& excluded);