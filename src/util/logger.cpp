#include "logger.h"

#include <fstream>
#include <iostream>
#include <algorithm> // for std::sort

void initialize_logger()
{
    Logger::timed_out = false;
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
    Logger::num_reduced_vertices = 0;
    Logger::num_reduced_edges = 0;
    Logger::cnt_dominated_vertices = 0;
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
    Logger::execution_time_nice_tree_decomposition = 0;
    Logger::execution_time_alber_rule_l = 0;

    //treewidth specific.
    Logger::maximum_treewidth = 0;
    Logger::execution_time_introduce = 0;
    Logger::execution_time_forget = 0;
    Logger::execution_time_join = 0;
    Logger::execution_time_introduce_edge = 0;
    Logger::execution_time_leaf = 0;

    Logger::average_SAT_execution_time.clear();

    Logger::num_introduce_bags_size.assign(Logger::num_introduce_bags_size.size(), 0);
    Logger::num_join_bags_size.assign(Logger::num_join_bags_size.size(), 0);
    Logger::num_forget_bags_size.assign(Logger::num_forget_bags_size.size(), 0);
    Logger::num_introduce_edge_bags_size.assign(Logger::num_introduce_edge_bags_size.size(), 0);
    Logger::treewidth.clear();
    Logger::num_vertices_components.clear();

    Logger::domination_number = 0;

    Logger::cnt_alber_l_reduction = 0;
    Logger::cnt_alber_l_either_reduction = 0;
    Logger::attempt_alber_l_reduction = 0;

    Logger::is_medium = false;
}

void initialize_logger_not_average()
{
    //Logger::timed_out = false;
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
    Logger::num_reduced_vertices = 0;
    Logger::num_reduced_edges = 0;
    Logger::cnt_dominated_vertices = 0;
    Logger::cnt_undetermined_vertices = 0;
    Logger::cnt_selected_vertices = 0;
    Logger::cnt_ignored_vertices = 0;
    Logger::cnt_excluded_vertices = 0;
    Logger::cnt_removed_vertices = 0;
    Logger::num_vertices_components.clear();

    //timer functions.
    Logger::execution_time_complete = 0;
    Logger::execution_time_treewidth = 0;
    Logger::execution_time_reduction = 0;
    Logger::execution_time_ilp = 0;
    Logger::execution_time_sat = 0;
    Logger::execution_time_nice_tree_decomposition = 0;
    Logger::execution_time_alber_rule_l = 0;

    //treewidth specific.
    Logger::maximum_treewidth = 0;
    Logger::execution_time_introduce = 0;
    Logger::execution_time_forget = 0;
    Logger::execution_time_join = 0;
    Logger::execution_time_introduce_edge = 0;
    Logger::execution_time_leaf = 0;

    Logger::num_introduce_bags_size.assign(Logger::num_introduce_bags_size.size(), 0);
    Logger::num_join_bags_size.assign(Logger::num_join_bags_size.size(), 0);
    Logger::num_forget_bags_size.assign(Logger::num_forget_bags_size.size(), 0);
    Logger::num_introduce_edge_bags_size.assign(Logger::num_introduce_edge_bags_size.size(), 0);
    Logger::treewidth.clear();

    Logger::domination_number = 0;

    Logger::cnt_alber_l_reduction = 0;
    Logger::cnt_alber_l_either_reduction = 0;
    Logger::attempt_alber_l_reduction = 0;

    Logger::is_medium = false;
}

    bool Logger::timed_out = false;

    int Logger::cnt_alber_simple_rule_1 = 0;
    int Logger::attempt_alber_simple_rule_1 = 0;

    int Logger::cnt_alber_simple_rule_2 = 0;
    int Logger::attempt_alber_simple_rule_2 = 0;

    int Logger::cnt_alber_simple_rule_3dot1 = 0;
    int Logger::cnt_alber_simple_rule_3dot2 = 0;
    int Logger::attempt_alber_simple_rule_3 = 0;

    int Logger::cnt_alber_simple_rule_4 = 0;
    int Logger::attempt_alber_simple_rule_4 = 0;

    int Logger::cnt_alber_rule_1_default = 0;
    int Logger::cnt_alber_rule_1_guard = 0;
    int Logger::attempt_alber_rule_1 = 0;

    int Logger::cnt_alber_rule_2_single = 0;
    int Logger::cnt_alber_rule_2_either = 0;
    int Logger::cnt_alber_rule_2_both = 0;
    int Logger::attempt_alber_rule_2 = 0;

    int Logger::cnt_ijcai_rule_1 = 0;
    int Logger::attempt_ijcai_rule_1 = 0;

    int Logger::cnt_ijcai_rule_2 = 0;
    int Logger::attempt_ijcai_rule_2 = 0;

    int Logger::cnt_ijcai_rule_3 = 0;
    int Logger::attempt_ijcai_rule_3 = 0;

    int Logger::cnt_alber_l_reduction = 0;
    int Logger::cnt_alber_l_either_reduction = 0;
    long long Logger::attempt_alber_l_reduction = 0;


    //components.
    int Logger::num_components = 0;
    int Logger::num_reduced_components = 0;
    std::vector<int> Logger::num_vertices_components;

    //effectiveness reduction rules.
    int Logger::num_vertices = 0;
    int Logger::num_edges = 0;
    int Logger::num_reduced_vertices = 0;
    int Logger::num_reduced_edges = 0;
    int Logger::cnt_undetermined_vertices = 0;
    int Logger::cnt_selected_vertices = 0;
    int Logger::cnt_dominated_vertices = 0;
    int Logger::cnt_ignored_vertices = 0;
    int Logger::cnt_excluded_vertices = 0;
    int Logger::cnt_removed_vertices = 0;

    //timer functions.
    long long Logger::execution_time_complete = 0;
    long long  Logger::execution_time_treewidth = 0;
    long long  Logger::execution_time_reduction = 0;
    long long Logger::execution_time_ilp = 0;
    long long  Logger::execution_time_sat = 0;
    long long Logger::execution_time_nice_tree_decomposition = 0;
    long long Logger::execution_time_alber_rule_l = 0;

    long long Logger::execution_time_seperate = 0;
    long long Logger::execution_dominations = 0;
    long long Logger::execution_alternative_dominations = 0;
    long long Logger:: execution_is_stronger = 0;

    std::vector<long long> Logger::average_SAT_execution_time;

    //treewidth specific.
    int Logger::maximum_treewidth = 0;
    long long Logger::execution_time_introduce = 0;
    long long Logger::execution_time_forget = 0;
    long long Logger::execution_time_join = 0;
    long long Logger::execution_time_introduce_edge = 0;
    long long Logger::execution_time_leaf = 0;

    std::vector<int> Logger::num_join_bags_size(20,0);
    std::vector<int> Logger::num_forget_bags_size(20, 0);
    std::vector<int> Logger::num_introduce_edge_bags_size(20, 0);
    std::vector<int> Logger::num_introduce_bags_size(20, 0);
    std::vector<int> Logger::treewidth;

    //strategy.
    strategy_reduction Logger::reduction_strategy = REDUCTION_COMBINATION;
    strategy_solver Logger::solver_strategy = SOLVER_COMBINATION;
    strategy_reduction_scheme Logger::reduction_scheme_strategy = REDUCTION_ALBER_L_NON;

    int Logger::domination_number = 0;

    bool Logger::is_medium = false;

std::string getReductionString(strategy_reduction reduction) {
    switch (reduction) {
    case REDUCTION_COMBINATION:
        return "REDUCTION_COMBINATION";
    case REDUCTION_ALBER:
        return "REDUCTION_ALBER";
    case REDUCTION_ALBER_RULE_1:
        return "REDUCTION_ALBER_RULE_1";
    case REDUCTION_IJCAI:
        return "REDUCTION_IJCAI";
    case REDUCTION_NON:
        return "REDUCTION_NON";
    default:
        throw std::runtime_error("Unknown reduction");
    }
}

std::string getSolverString(strategy_solver reduction) {
    switch (reduction) {
    case SOLVER_SAT:
        return "SOLVER_SAT";
    case SOLVER_ILP:
        return "SOLVER_ILP";
    case SOLVER_NON:
        return "SOLVER_NON";
    case SOLVER_TREEWIDTH:
        return "SOLVER_TREEWIDTH";
    case SOLVER_COMBINATION:
        return "SOLVER_COMBINATION";
    case SOLVER_NICE_TREE_DECOMPOSITION:
        return "SOLVER_NICE_TREE_DECOMPOSITION";
    default:
        throw std::runtime_error("Unknown reduction");
    }
}

std::string getReductionSchemeString(strategy_reduction_scheme reduction) {
    switch (reduction) {
    case REDUCTION_ALBER_L_3:
        return "REDUCTION_ALBER_L_3";
    case REDUCTION_ALBER_L_4:
        return "REDUCTION_ALBER_L_4";
    case REDUCTION_ALBER_L_5:
        return "REDUCTION_ALBER_L_5";
    case REDUCTION_ALBER_L_NON:
        return "REDUCTION_ALBER_L_NON";
    default:
        throw std::runtime_error("Unknown reduction");
    }
}

void output_loginfo(std::string& name) {
    std::string prefix = "/home/floris/github/minimum-dominating-set/log_info/";
    //+ "/only_reduced" +
    std::string output_path = prefix + getSolverString(Logger::solver_strategy) + "/" + getReductionString(Logger::reduction_strategy) + "/" + getReductionSchemeString(Logger::reduction_scheme_strategy) + "/approx" + "/loginfo_" + name;
    std::cout << output_path << std::endl;
    //"/single_thread"
    std::ofstream outFile(output_path);

    if (!outFile) {
        printf("error with writing to file");
        return;
    }
    //strategy
    outFile << "Timed out: " << Logger::timed_out << std::endl;
    outFile << "Reduction strategy: " << getReductionString(Logger::reduction_strategy) << std::endl;
    outFile << "Solver strategy: " << getSolverString(Logger::solver_strategy) << std::endl;
    outFile << "Reduction scheme strategy: " << getReductionSchemeString(Logger::reduction_scheme_strategy) << std::endl;
    //Timer
    outFile << "Execution time complete: " << Logger::execution_time_complete << std::endl;
    outFile << "Execution time reduction: " << Logger::execution_time_reduction << std::endl;
    outFile << "Domination number: " << Logger::domination_number << std::endl;
    if (Logger::solver_strategy != SOLVER_NON)
    {
        std::sort(Logger::num_vertices_components.begin(), Logger::num_vertices_components.end(), std::greater<int>());
        outFile << "vertices components:" << std::endl;

        for (int size : Logger::num_vertices_components)
        {
            outFile << size << " ";
        }
        outFile << std::endl;
        if (Logger::solver_strategy == SOLVER_COMBINATION) {
            outFile << "Execution time ILP: " << Logger::execution_time_ilp << std::endl;
            outFile << "Execution time SAT: " << Logger::execution_time_sat << std::endl;
            outFile << "Execution time treewidth: " << Logger::execution_time_treewidth << std::endl;
        } else if (Logger::solver_strategy == SOLVER_SAT){
            outFile << "Execution time SAT: " << Logger::execution_time_sat << std::endl;
            outFile << "num average: " << Logger::average_SAT_execution_time.size() << std::endl;
            if (!Logger::average_SAT_execution_time.empty()) {
                double sum = std::accumulate(Logger::average_SAT_execution_time.begin(), Logger::average_SAT_execution_time.end(), 0.0);
                double average = sum / Logger::average_SAT_execution_time.size();
                outFile << "Average SAT_execution: " << average << std::endl;
            } else {
                outFile << "Vector is empty." << std::endl;
            }

        } else if (Logger::solver_strategy == SOLVER_ILP) {
            outFile << "Execution time ILP: " << Logger::execution_time_ilp << std::endl;
        } else if (Logger::solver_strategy == SOLVER_TREEWIDTH){ // Extra information only if you run treewidth specifically.
            outFile << "Execution time treewidth: " << Logger::execution_time_treewidth << std::endl;
            outFile << "maximum_treewidth : " << Logger::maximum_treewidth << std::endl;
            outFile << "Execution time spent in join: " << Logger::execution_time_join << std::endl;
            outFile << "Execution time spent in leaf: " << Logger::execution_time_leaf << std::endl;
            outFile << "Execution time spent in introduce: " << Logger::execution_time_introduce << std::endl;
            outFile << "Execution time spent in introduce edge: " << Logger::execution_time_introduce_edge << std::endl;
            outFile << "Execution time spent in forget: " << Logger::execution_time_forget << std::endl;
            outFile << "Treewidths of different components: ";
            std::sort(Logger::treewidth.begin(), Logger::treewidth.end());
            for (int j : Logger::treewidth){
                outFile << j << " ";
            }
            outFile << std::endl;
            for (size_t i = 0; i < Logger::num_join_bags_size.size(); ++i) {
                outFile << "cnt Bag size join" << i << ": " << Logger::num_join_bags_size[i] << std::endl;
            }
            for (size_t i = 0; i < Logger::num_introduce_bags_size.size(); ++i) {
                outFile << "cnt Bag size introduce" << i << ": " << Logger::num_introduce_bags_size[i] << std::endl;
            }
            for (size_t i = 0; i < Logger::num_introduce_edge_bags_size.size(); ++i) {
                outFile << "cnt Bag size introduce_edge" << i << ": " << Logger::num_introduce_edge_bags_size[i] << std::endl;
            }
            for (size_t i = 0; i < Logger::num_forget_bags_size.size(); ++i) {
                outFile << "cnt Bag size forget" << i << ": " << Logger::num_forget_bags_size[i] << std::endl;
            }
        } else if (Logger::solver_strategy == SOLVER_NICE_TREE_DECOMPOSITION) {
            outFile << "Execution time of the nice tree decompositions: " << Logger::execution_time_nice_tree_decomposition << std::endl;
            outFile << "maximum_treewidth : " << Logger::maximum_treewidth << std::endl;
            outFile << "Treewidths of different components: ";
            std::sort(Logger::treewidth.begin(), Logger::treewidth.end(), std::greater<int>());
            for (int j : Logger::treewidth){
                outFile << j << " ";
            }
            outFile << std::endl;
        }
        else {
            throw std::runtime_error("non-supported solver strategy.");
        }
    }
    //effectiveness reduction rules.
    outFile << "Number of vertices: " << Logger::num_vertices << std::endl;
    outFile << "Number of edges: " << Logger::num_edges << std::endl;
    outFile << "Number of reduced_vertices: " << Logger::num_reduced_vertices << std::endl;
    outFile << "Number of reduced_edges: " << Logger::num_reduced_edges << std::endl;
    outFile << "Number of undetermined vertices: " << Logger::cnt_undetermined_vertices << std::endl;
    outFile << "Number of dominated vertices: " << Logger::cnt_dominated_vertices << std::endl;
    outFile << "Number of selected vertices: " << Logger::cnt_selected_vertices << std::endl;
    outFile << "Number of ignored vertices: "  << Logger::cnt_ignored_vertices << std::endl;
    outFile << "Number of excluded vertices: " << Logger::cnt_excluded_vertices << std::endl;
    outFile << "Number of removed vertices: " << Logger::cnt_removed_vertices << std::endl;

    //Components:
    outFile << "Number of components: " << Logger::num_components << std::endl;
    if (Logger::reduction_strategy != REDUCTION_NON){
        outFile << "Number of components after reduction: " << Logger::num_reduced_components << std::endl;
    }

    if (Logger::solver_strategy == SOLVER_NON){ // Extra information only if you run reductions only.
        if (Logger::reduction_strategy == REDUCTION_COMBINATION) {
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
        } else if (Logger::reduction_strategy == REDUCTION_ALBER || Logger::reduction_strategy == REDUCTION_ALBER_RULE_1) {
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

            if (Logger::reduction_strategy == REDUCTION_ALBER) {
                outFile << "Attempts of Alber rule 2: " << Logger::attempt_alber_rule_2 << std::endl;
                outFile << "Successful reduction of Alber rule 2 (single): " << Logger::cnt_alber_rule_2_single << std::endl;
                outFile << "Successful reduction of alber rule 2 (either): " << Logger::cnt_alber_rule_2_either << std::endl;
                outFile << "Successful reduction of alber rule 2 (both): " << Logger::cnt_alber_rule_2_both << std::endl;
            }
        } else if (Logger::reduction_strategy == REDUCTION_IJCAI) {
            outFile << "Attempts of IJCAI rule 1: " << Logger::attempt_ijcai_rule_1 << std::endl;
            outFile << "Successful reduction of IJCAI rule 1: " << Logger::cnt_ijcai_rule_1 << std::endl;

            outFile << "Attempts of IJCAI rule 2: " << Logger::attempt_ijcai_rule_2 << std::endl;
            outFile << "Successful reduction of rule 2: " << Logger::cnt_ijcai_rule_2 << std::endl;

            outFile << "Attempts of rule 3: " << Logger::attempt_ijcai_rule_3 << std::endl;
            outFile << "Successful reduction of rule 3: " << Logger::cnt_ijcai_rule_3 << std::endl;
        } else if (Logger::reduction_strategy == REDUCTION_NON){
        } else {
            throw std::runtime_error("non-supported reduction strategy.");
        }
        if (Logger::reduction_scheme_strategy != REDUCTION_ALBER_L_NON){
            outFile << "Execution time of ALBER rule l: " << Logger::execution_time_alber_rule_l << std::endl;
            outFile << "Attempts of ALBER rule l : " << Logger::attempt_alber_l_reduction << std::endl;
            outFile << "Successful reduction of ALBER rule l: " << Logger::cnt_alber_l_reduction << std::endl;
            outFile << "Successful reduction of ALBER rule l (either): " << Logger::cnt_alber_l_either_reduction << std::endl;
        }
    }

    outFile.close();
}