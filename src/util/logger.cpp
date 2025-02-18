#include "logger.h"

#include <fstream>

void initialize_logger() {
    Logger::flag_sr_1 = true;
    Logger::flag_sr_2 = true;
    Logger::flag_sr_3 = true;
    Logger::flag_sr_4 = true;
    Logger::flag_neigh_single = true;
    Logger::flag_neigh_pair = true;

    // Initialize static integer counters
// Initialize static integer counters
    Logger::cnt_simple_rule_one = 0;
    Logger::cnt_simple_rule_two = 0;
    Logger::cnt_simple_rule_three_dot_one = 0;
    Logger::cnt_simple_rule_three_dot_two = 0;
    Logger::cnt_simple_rule_four = 0;
    Logger::cnt_reduce_neighborhood_single_vertex = 0;
    Logger::cnt_reduce_neighborhood_single_vertex_guard = 0;
    Logger::cnt_reduce_neighborhood_pair_vertex_single = 0; //can be dominated by only v or only w
    Logger::cnt_reduce_neighborhood_pair_vertex_both = 0; //dont know whether to choose v or w
    Logger::cnt_reduce_neighborhood_pair_vertex_either = 0; //has to be dominated by both v and w.

    Logger::att_simple_rule_one = 0;
    Logger::att_simple_rule_two = 0;
    Logger::att_simple_rule_three = 0;
    Logger::att_simple_rule_four = 0;
    Logger::att_reduce_neighborhood_single_vertex = 0;
    Logger::att_reduce_neighborhood_pair_vertex = 0;


    Logger::no_undetermined_vertices = false;
    Logger::solution_vector_with_reduction = {};
    Logger::solution_vector_without_reduction = {};

    // Initialize static long long execution times
    Logger::execution_ilp_without_reduction = 0;
    Logger::execution_ilp_with_reduction = 0;
    Logger::execution_reduction = 0;

    Logger::is_planar = true; //temp value, gets updated
}
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
int Logger::cnt_simple_rule_three_dot_one = 0;
int Logger::cnt_simple_rule_three_dot_two = 0;
int Logger::cnt_simple_rule_four = 0;
int Logger::cnt_reduce_neighborhood_single_vertex = 0;
int Logger::cnt_reduce_neighborhood_single_vertex_guard = 0;
int Logger::cnt_reduce_neighborhood_pair_vertex_single = 0; //can be dominated by only v or only w
int Logger::cnt_reduce_neighborhood_pair_vertex_both =  0; //dont know whether to choose v or w
int Logger::cnt_reduce_neighborhood_pair_vertex_either = 0; //has to be dominated by both v and w.

int Logger::att_simple_rule_one = 0;
int Logger::att_simple_rule_two = 0;
int Logger::att_simple_rule_three = 0;
int Logger::att_simple_rule_four = 0;
int Logger::att_reduce_neighborhood_single_vertex = 0;
int Logger::att_reduce_neighborhood_pair_vertex = 0;


bool Logger::no_undetermined_vertices = false;
std::vector<int> Logger::solution_vector_with_reduction = {};
std::vector<int> Logger::solution_vector_without_reduction = {};

// Initialize static long long execution times
long long Logger::execution_ilp_without_reduction = 0;
long long Logger::execution_ilp_with_reduction = 0;
long long Logger::execution_reduction = 0;

bool Logger::is_planar = true; //temp value, gets updated

void output_loginfo(std::string& name, std::vector<int>& included, std::vector<int>& dominated, std::vector<int>& removed, std::vector<int>& ignored) {
    std::string prefix = "/mnt/c/Users/Flori/OneDrive/Documenten/GitHub/minimum-dominating-set/log_info/loginfo_";
    std::string output_path = prefix + name;

    std::ofstream outFile(output_path);

    if (!outFile) {
        printf("error with writing to file");
        return;
    }

    //outFile << "Execution ILP with reduction: " << Logger::execution_ilp_with_reduction << " milliseconds (" << (Logger::execution_ilp_with_reduction / 1000.0) << " seconds)" << std::endl;
    //outFile << "Execution ILP without reduction: " << Logger::execution_ilp_without_reduction << " milliseconds (" << (Logger::execution_ilp_without_reduction / 1000.0) << " seconds)" << std::endl;
    outFile << "flags: flag_simple_rule_one:" << Logger::flag_sr_1 << " flag_simple_rule_2:" << Logger::flag_sr_2 << " flag_simple_rule_3:" << Logger::flag_sr_3 << " flag_simple_rule_4:" << Logger::flag_sr_4 << " flag_neigh_single:" << Logger::flag_neigh_single << " flag_neigh_pair:" << Logger::flag_neigh_pair << std::endl;
    outFile << "execution reduction rules:" << Logger::execution_reduction << " milliseconds (" << (Logger::execution_reduction / 1000.0) << " seconds)" << std::endl;
    outFile << "Attempts of rule_1: " << Logger::att_simple_rule_one << " | Successful rule_1 reductions: " << Logger::cnt_simple_rule_one << std::endl;
    outFile << "Attempts of rule_2: " << Logger::att_simple_rule_two << " | Successful rule_2 reductions: " << Logger::cnt_simple_rule_two << std::endl;
    outFile << "Attempts of rule_3: " << Logger::att_simple_rule_three << " | Successful rule_3.1 reductions: " << Logger::cnt_simple_rule_three_dot_one << " | Successful rule_3.2 reductions: " << Logger::cnt_simple_rule_three_dot_two << std::endl;
    outFile << "Attempts of rule_4: " << Logger::att_simple_rule_four << " | Successful rule_4 reductions: " << Logger::cnt_simple_rule_four << std::endl;

    outFile << "Attempts to reduce neighborhood (single vertex): " << Logger::att_reduce_neighborhood_single_vertex
        << " | Successful neighborhood reductions (single vertex): " << Logger::cnt_reduce_neighborhood_single_vertex
        << " | Successful neighborhood reductions (guard): " << Logger::cnt_reduce_neighborhood_single_vertex_guard << std::endl;
    outFile << "Attempts to reduce neighborhood (pair vertex): " << Logger::att_reduce_neighborhood_pair_vertex 
        << " | Successful neighborhood reductions (pair vertex): single " << Logger::cnt_reduce_neighborhood_pair_vertex_single 
        << " | Successful neighborhood reductions (pair vertex): both " << Logger::cnt_reduce_neighborhood_pair_vertex_both
        << " | Successful neighborhood reductions (pair vertex): either " << Logger::cnt_reduce_neighborhood_pair_vertex_either << std::endl;
   
    outFile << "number of dominated vertices " << dominated.size() << " index dominated verticies: ";
    for (int index : dominated) {
            outFile << index << " ";
    }
    outFile << std::endl;

    outFile << "number of included vertices " << included.size() << " index included verticies: ";
    for (int index : included) {
        outFile << index << " ";
    }
    outFile << std::endl;

    outFile << "number of removed vertices " << removed.size() << " index removed verticies: ";
    for (int index : removed) {
        outFile << index << " ";
    }
    outFile << std::endl;

    outFile << "number of ignored vertices " << ignored.size() << " index ignored verticies: ";
    for (int index : ignored) {
        outFile << index << " ";
    }
    outFile << std::endl;
    
    
    outFile << "is planer? " << Logger::is_planar << std::endl;
    outFile << "no undetermined vertices: " << Logger::no_undetermined_vertices << std::endl;
    outFile << "Size Optimal solution with reduction rules:" << Logger::solution_vector_with_reduction.size() << std::endl;
    outFile << "optimal solution with reduction rules: ";
    for (int num : Logger::solution_vector_with_reduction) {
        outFile << num << " ";
    }
    outFile << std::endl;
    outFile << "Size optimal solution without reduction rules:" << Logger::solution_vector_without_reduction.size() << std::endl;
    outFile << "optimal solution without reduction rules: ";
    for (int num : Logger::solution_vector_without_reduction) {
        outFile << num << " ";
    }

    outFile.close();
}