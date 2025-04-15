#pragma once
#include <string>
#include <vector>

void initialize_logger();

enum strategy_solver{
	SOLVER_COMBINATION,
	SOLVER_ILP,
	SOLVER_SAT,
	SOLVER_TREEWIDTH,
	SOLVER_NICE_TREE_DECOMPOSITION,
	SOLVER_NON
};

enum strategy_reduction{
	REDUCTION_COMBINATION,
	REDUCTION_ALBER,
	REDUCTION_ALBER_RULE_1,
	REDUCTION_IJCAI,
	REDUCTION_NON
};

class Logger {
public:
	//reduction rules.
	static int cnt_alber_simple_rule_1;
	static int attempt_alber_simple_rule_1;

	static int cnt_alber_simple_rule_2;
	static int attempt_alber_simple_rule_2;

	static int cnt_alber_simple_rule_3dot1;
	static int cnt_alber_simple_rule_3dot2;
	static int attempt_alber_simple_rule_3;

	static int cnt_alber_simple_rule_4;
	static int attempt_alber_simple_rule_4;

	static int cnt_alber_rule_1_default;
	static int cnt_alber_rule_1_guard;
	static int attempt_alber_rule_1;

	static int cnt_alber_rule_2_single;
	static int cnt_alber_rule_2_either;
	static int cnt_alber_rule_2_both;
	static int attempt_alber_rule_2;

	static int cnt_ijcai_rule_1;
	static int attempt_ijcai_rule_1;

	static int cnt_ijcai_rule_2;
	static int attempt_ijcai_rule_2;

	static int cnt_ijcai_rule_3;
	static int attempt_ijcai_rule_3;

	//components.
	static int num_components;
	static int num_reduced_components;

	//effectiveness reduction rules.
	static int num_vertices;
	static int num_edges;
	static int num_reduced_vertices;
	static int num_reduced_edges;
	static int cnt_undetermined_vertices;
	static int cnt_selected_vertices;
	static int cnt_ignored_vertices;
	static int cnt_excluded_vertices;
	static int cnt_removed_vertices;

	//timer functions.
	static long long execution_time_complete;
	static long long execution_time_treewidth;
	static long long execution_time_reduction;
	static long long execution_time_ilp;
	static long long execution_time_sat;
	static long long execution_time_nice_tree_decomposition;

	//treewidth specific.
	static int maximum_treewidth;
	static long long execution_time_introduce;
	static long long execution_time_forget;
	static long long execution_time_join;
	static long long execution_time_introduce_edge;
	static long long execution_time_leaf;

	//strategy.
	static strategy_reduction reduction_strategy;
	static strategy_solver solver_strategy;
};

void output_loginfo(std::string& name);