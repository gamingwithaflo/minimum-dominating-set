#include "logger.h"

// Initialize static boolean flags
bool Logger::flag_sr_1 = true;
bool Logger::flag_sr_2 = true;
bool Logger::flag_sr_3 = true;
bool Logger::flag_sr_4 = true;
bool Logger::flag_neigh_single = true;
bool Logger::flag_neigh_pair = true;

// Initialize static integer counters
int Logger::cnt_simple_rule_one = 0;
int Logger::cnt_simple_rule_two = 0;
int Logger::cnt_simple_rule_three = 0;
int Logger::cnt_simple_rule_four = 0;
int Logger::cnt_reduce_neighborhood_single_vertex = 0;
int Logger::cnt_reduce_neighborhood_pair_vertex = 0;

int Logger::att_simple_rule_one = 0;
int Logger::att_simple_rule_two = 0;
int Logger::att_simple_rule_three = 0;
int Logger::att_simple_rule_four = 0;
int Logger::att_reduce_neighborhood_single_vertex = 0;
int Logger::att_reduce_neighborhood_pair_vertex = 0;

// Initialize static long long execution times
long long Logger::execution_ilp_without_reduction = 0;
long long Logger::execution_ilp_with_reduction = 0;
long long Logger::execution_reduction = 0;

bool Logger::is_planar = true; //temp value, gets updated

void Logger::output_log(std::string path) {

}