#pragma once
#include <string>
#include <vector>
#include <numeric>

void initialize_logger();

void initialize_logger_not_average();

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
	REDUCTION_NON,
	REDUCTION_L_ALBER,
};

enum strategy_reduction_scheme{
	REDUCTION_ALBER_L_3,
	REDUCTION_ALBER_L_4,
	REDUCTION_ALBER_L_5,
	REDUCTION_ALBER_L_NON,
};

class Logger {
public:
	//reduction rules.
	static bool timed_out;

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

	static int cnt_alber_l_reduction;
	static int cnt_alber_l_either_reduction;
	static long long attempt_alber_l_reduction;

	//components.
	static int num_components;
	static int num_reduced_components;
	static std::vector<int> num_vertices_components;

	//effectiveness reduction rules.
	static int num_vertices;
	static int num_edges;
	static int num_reduced_vertices;
	static int num_reduced_edges;
	static int cnt_undetermined_vertices;
	static int cnt_selected_vertices;
	static int cnt_dominated_vertices;
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
	static long long execution_time_alber_rule_l;

	static long long execution_time_seperate;
	static long long execution_dominations;
	static long long execution_alternative_dominations;
	static long long execution_is_stronger;

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
	static strategy_reduction_scheme reduction_scheme_strategy;

	static std::vector<long long> average_SAT_execution_time;

	static std::vector<int> num_join_bags_size;
	static std::vector<int> num_forget_bags_size;
	static std::vector<int> num_introduce_edge_bags_size;
	static std::vector<int> num_introduce_bags_size;

	static std::vector<int> treewidth;

	static int domination_number;
};

void output_loginfo(std::string& name);