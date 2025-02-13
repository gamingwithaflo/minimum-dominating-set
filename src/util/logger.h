#pragma once
#include <string>

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
	static int cnt_simple_rule_three;
	static int cnt_simple_rule_four;
	static int cnt_reduce_neighborhood_single_vertex;
	static int cnt_reduce_neighborhood_pair_vertex;

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