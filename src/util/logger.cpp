#include "logger.h"

#include <fstream>

void initialize_logger()
{
    //reduction rules.
    Logger::cnt_alber_simple_rule_1 = 0;
    Logger::attempt_alber_simple_rule_1 = 0;

    Logger::cnt_alber_simple_rule_2 = 0;
    Logger::attempt_alber_simple_rule_2 = 0;

    Logger::cnt_alber_simple_rule_3dot1 = 0;
    Logger::cnt_alber_simple_rule_3dot2 = 0;
    Logger::attempt_alber_simple_rule_3 = 0;

    Logger::cnt_alber_simple_rule_4 = 0;
    Logger::attempt_alber_simple_rule_4 = 0;

    Logger::cnt_alber_rule_1_default = 0;
    Logger::cnt_alber_rule_1_guard = 0;
    Logger::attempt_alber_rule_1 = 0;

    Logger::cnt_alber_rule_2_single = 0;
    Logger::cnt_alber_rule_2_either = 0;
    Logger::cnt_alber_rule_2_both = 0;
    Logger::attempt_alber_rule_2 = 0;

    Logger::cnt_ijcai_rule_1 = 0;
    Logger::attempt_ijcai_rule_1 = 0;

    Logger::cnt_ijcai_rule_2 = 0;
    Logger::attempt_ijcai_rule_2 = 0;

    Logger::cnt_ijcai_rule_3 = 0;
    Logger::attempt_ijcai_rule_3 = 0;

    //components.
    Logger::num_components = 0;
    Logger::num_reduced_components = 0;

    //effectiveness reduction rules.
    Logger::num_vertices = 0;
    Logger::num_edges = 0;
    Logger::cnt_undetermined_vertices = 0;
    Logger::cnt_selected_vertices = 0;
    Logger::cnt_ignored_vertices = 0;
    Logger::cnt_excluded_vertices = 0;
    Logger::cnt_removed_vertices = 0;

    //timer functions.
    Logger::execution_time_complete = 0;
    Logger::execution_time_treewidth = 0;
    Logger::execution_time_reduction = 0;
    Logger::execution_time_ilp = 0;
    Logger::execution_time_sat = 0;

    //treewidth specific.
    Logger::maximum_treewidth = 0;
    Logger::execution_time_introduce = 0;
    Logger::execution_time_forget = 0;
    Logger::execution_time_join = 0;
    Logger::execution_time_introduce_edge = 0;
    Logger::execution_time_leaf = 0;

    //strategy.
    static std::string reduction_strategy = "Combination";
    static std::string solver_strategy = "Combination";
}

void output_loginfo(std::string& name, std::vector<int>& included, std::vector<int>& dominated, std::vector<int>& removed, std::vector<int>& ignored, std::vector<int>& excluded) {
    std::string prefix = "/mnt/c/Users/Flori/OneDrive/Documenten/GitHub/minimum-dominating-set/log_info/loginfo_";
    std::string output_path = prefix + name;

    std::ofstream outFile(output_path);

    if (!outFile) {
        printf("error with writing to file");
        return;
    }
    //strategy
    outFile << "Reduction strategy: " << Logger::reduction_strategy << std::endl;
    outFile << "Solver strategy: " << Logger::solver_strategy << std::endl;
    //Timer
    outFile << "Execution time complete: " << Logger::execution_time_complete << std::endl;
    outFile << "Execution time reduction: " << Logger::execution_time_reduction << std::endl;
    if (Logger::solver_strategy != "non")
    {
        if (Logger::solver_strategy == "Combination") {
            outFile << "Execution time ILP: " << Logger::execution_time_ilp << std::endl;
            outFile << "Execution time SAT: " << Logger::execution_time_sat << std::endl;
            outFile << "Execution time treewidth: " << Logger::execution_time_treewidth << std::endl;
        } else if (Logger::solver_strategy == "sat"){
            outFile << "Execution time SAT: " << Logger::execution_time_sat << std::endl;
        } else if (Logger::solver_strategy == "ilp") {
            outFile << "Execution time ILP: " << Logger::execution_time_ilp << std::endl;
        } else if (Logger::solver_strategy == "treewidth"){ // Extra information only if you run treewidth specifically.
            outFile << "Execution time treewidth: " << Logger::execution_time_treewidth << std::endl;
            outFile << "Execution time spent in join: " << Logger::execution_time_join << std::endl;
            outFile << "Execution time spent in leaf: " << Logger::execution_time_leaf << std::endl;
            outFile << "Execution time spent in introduce: " << Logger::execution_time_introduce << std::endl;
            outFile << "Execution time spent in introduce edge: " << Logger::execution_time_introduce_edge << std::endl;
            outFile << "Execution time spent in forget: " << Logger::execution_time_forget << std::endl;
        } else {
            throw std::runtime_error("non-supported solver strategy.");
        }
    }
    //effectiveness reduction rules.
    outFile << "Number of vertices: " << Logger::num_vertices << std::endl;
    outFile << "Number of edges: " << Logger::num_edges << std::endl;
    outFile << "Number of undetermined vertices: " << Logger::cnt_undetermined_vertices << std::endl;
    outFile << "Number of selected vertices: " << Logger::cnt_selected_vertices << std::endl;
    outFile << "Number of ignored vertices: "  << Logger::cnt_ignored_vertices << std::endl;
    outFile << "Number of excluded vertices: " << Logger::cnt_excluded_vertices << std::endl;
    outFile << "Number of removed vertices: " << Logger::cnt_removed_vertices << std::endl;

    //Components:
    outFile << "Number of components: " << Logger::num_components << std::endl;
    if (Logger::reduction_strategy != "non"){
        outFile << "Number of components after reduction: " << Logger::num_reduced_components << std::endl;
    }

    if (Logger::solver_strategy == "non"){ // Extra information only if you run reductions only.
        if (Logger::reduction_strategy == "Combination") {
            outFile << "Attempts of rule_1: " << Logger::attempt_ijcai_rule_1 << std::endl;
            outFile << "Successful reduction of IJCAI rule 1: " << Logger::cnt_ijcai_rule_1 << std::endl;

            outFile << "Attempts of rule_2: " << Logger::attempt_ijcai_rule_2 << std::endl;
            outFile << "Successful reduction of IJCAI rule 2: " << Logger::cnt_ijcai_rule_2 << std::endl;

            outFile << "Attempts of rule_3: " << Logger::attempt_ijcai_rule_3 << std::endl;
            outFile << "Successful reduction of IJCAI rule 3: " << Logger::cnt_ijcai_rule_3 << std::endl;

            outFile << "Attempts of Alber rule 2: " << Logger::attempt_alber_rule_2 << std::endl;
            outFile << "Successful reduction of Alber rule 2 (single): " << Logger::cnt_alber_rule_2_single << std::endl;
            outFile << "Successful reduction of alber rule 2 (either): " << Logger::cnt_alber_rule_2_either << std::endl;
            outFile << "Successful reduction of alber rule 2 (both): " << Logger::cnt_alber_rule_2_both << std::endl;
        } else if (Logger::reduction_strategy == "Alber"){
            outFile << "Attempts of alber simple rule 1: " << Logger::attempt_alber_simple_rule_1 << std::endl;
            outFile << "Successful reduction of alber simple rule 1: " << Logger::cnt_alber_simple_rule_1 << std::endl;

            outFile << "Attempts of alber simple rule 2: " << Logger::attempt_alber_simple_rule_2 << std::endl;
            outFile << "Successful reduction of alber simple rule 2: " << Logger::cnt_alber_simple_rule_2 << std::endl;

            outFile << "Attempts of alber simple rule 3: " << Logger::attempt_alber_simple_rule_3 << std::endl;
            outFile << "Successful reduction of alber simple rule 3.1: " << Logger::cnt_alber_simple_rule_3dot1 << std::endl;
            outFile << "Successful reduction of alber simple rule 3.2: " << Logger::cnt_alber_simple_rule_3dot2 << std::endl;

            outFile << "Attempts of alber simple rule 4: " << Logger::attempt_alber_simple_rule_4 << std::endl;
            outFile << "Successful reduction of alber simple rule 4: " << Logger::cnt_alber_simple_rule_4 << std::endl;

            outFile << "Attempts of alber rule 1: " << Logger::attempt_alber_rule_1 << std::endl;
            outFile << "Successful reduction alber rule 1 default: " << Logger::cnt_alber_rule_1_default << std::endl;
            outFile << "Successful reduction alber rule 1 guard: " << Logger::cnt_alber_rule_1_guard << std::endl;

            outFile << "Attempts of Alber rule 2: " << Logger::attempt_alber_rule_2 << std::endl;
            outFile << "Successful reduction of Alber rule 2 (single): " << Logger::cnt_alber_rule_2_single << std::endl;
            outFile << "Successful reduction of alber rule 2 (either): " << Logger::cnt_alber_rule_2_either << std::endl;
            outFile << "Successful reduction of alber rule 2 (both): " << Logger::cnt_alber_rule_2_both << std::endl;
        } else if (Logger::reduction_strategy == "IJCAI") {
            outFile << "Attempts of IJCAI rule 1: " << Logger::attempt_ijcai_rule_1 << std::endl;
            outFile << "Successful reduction of IJCAI rule 1: " << Logger::cnt_ijcai_rule_1 << std::endl;

            outFile << "Attempts of IJCAI rule 2: " << Logger::attempt_ijcai_rule_2 << std::endl;
            outFile << "Successful reduction of rule 2: " << Logger::cnt_ijcai_rule_2 << std::endl;

            outFile << "Attempts of rule 3: " << Logger::attempt_ijcai_rule_3 << std::endl;
            outFile << "Successful reduction of rule 3: " << Logger::cnt_ijcai_rule_3 << std::endl;
        } else if (Logger::reduction_strategy == "non"){
        } else {
            throw std::runtime_error("non-supported reduction strategy.");
        }
    }

    outFile.close();
}