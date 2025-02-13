#include "logger.h"

#include <fstream>

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

void output_loginfo(std::string& name) {
    std::string prefix = "/mnt/c/Users/Flori/OneDrive/Documenten/GitHub/minimum-dominating-set/log_info/loginfo_";
    std::string output_path = prefix + name;

    std::ofstream outFile(output_path);

    if (!outFile) {
        printf("error with writing to file");
        return;
    }

    outFile << "flags: flag_simple_rule_one:" << Logger::flag_sr_1 << " flag_simple_rule_2:" << Logger::flag_sr_2 << " flag_simple_rule_3:" << Logger::flag_sr_3 << " flag_simple_rule_4:" << Logger::flag_sr_4 << " flag_neigh_single:" << Logger::flag_neigh_single << " flag_neigh_pair:" << Logger::flag_neigh_pair << std::endl;

    outFile << "Execution ILP with reduction: " << Logger::execution_ilp_with_reduction << " milliseconds (" << (Logger::execution_ilp_with_reduction / 1000.0) << " seconds)" << std::endl;
    outFile << "Execution ILP without reduction: " << Logger::execution_ilp_without_reduction << " milliseconds (" << (Logger::execution_ilp_without_reduction / 1000.0) << " seconds)" << std::endl;
    outFile << "execution reduction rules:" << Logger::execution_reduction << " milliseconds (" << (Logger::execution_reduction / 1000.0) << " seconds)" << std::endl;
    
    outFile << "Attempts of rule_1: " << Logger::att_simple_rule_one << " | Successful rule_1 reductions: " << Logger::cnt_simple_rule_one << std::endl;
    outFile << "Attempts of rule_2: " << Logger::att_simple_rule_two << " | Successful rule_2 reductions: " << Logger::cnt_simple_rule_two << std::endl;
    outFile << "Attempts of rule_3: " << Logger::att_simple_rule_three << " | Successful rule_3 reductions: " << Logger::cnt_simple_rule_three << std::endl;
    outFile << "Attempts of rule_4: " << Logger::att_simple_rule_four << " | Successful rule_4 reductions: " << Logger::cnt_simple_rule_four << std::endl;

    outFile << "Attempts to reduce neighborhood (single vertex): " << Logger::att_reduce_neighborhood_single_vertex << " | Successful neighborhood reductions (single vertex): " << Logger::cnt_reduce_neighborhood_single_vertex << std::endl;
    outFile << "Attempts to reduce neighborhood (pair vertex): " << Logger::att_reduce_neighborhood_pair_vertex << " | Successful neighborhood reductions (pair vertex): " << Logger::cnt_reduce_neighborhood_pair_vertex << std::endl;

    outFile.close();
}