#include "sat_solver.h"
#include "ortools/sat/sat_solver.h"

#include <fstream>
#include <stdint.h>
#include <stdlib.h>

#include "util/timer.h"
#include <string>
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"
#include "util/logger.h"
#include "ortools/base/logging.h"

using namespace operations_research::sat;


std::vector<int> sat_solver_dominating_set(MDS_CONTEXT& mds_context, adjacencyListBoost& graph, std::unordered_map<int,int>& newToOldIndex){
    CpModelBuilder cp_model;

    std::unordered_map<int, int> translation_pace_to_ilp;
    std::unordered_map<int, int> translation_ilp_to_pace;
    //create decision variables.

    operations_research::Domain domain(0, 1);
    std::vector<IntVar> decision_vars;

    int index = 0;
    for (int i = 0; i < boost::num_vertices(graph); ++i)
    {
        if (mds_context.is_undetermined(newToOldIndex[i])){
            decision_vars.push_back(cp_model.NewIntVar(domain).WithName(std::to_string(i)));
            translation_pace_to_ilp[i] = index;
            translation_ilp_to_pace[index] = i;
            index++;
        }
    }

    if (decision_vars.empty()){
        //no undetermined vertices.
        return std::vector<int>();
    }

    //Create constraint.
    for (int i = 0; i < boost::num_vertices(graph); i++) { //|| mds_context.is_ignored(newToOldIndex[i])
        if (mds_context.is_dominated(newToOldIndex[i])){
            continue;
        }
        auto [neigh_itt, neigh_itt_end] = boost::adjacent_vertices(i, graph);
        std::vector<int> undetermined_neighbours;
        int num_neighbours = 0;
        undetermined_neighbours.reserve(std::distance(neigh_itt, neigh_itt_end));
        for (auto vertex = neigh_itt ; vertex != neigh_itt_end; ++vertex) {
            num_neighbours++;
            if (mds_context.is_undetermined(newToOldIndex[*vertex])){
                if (mds_context.is_removed(newToOldIndex[*vertex]))
                {
                    throw std::runtime_error("bad");
                }
                undetermined_neighbours.push_back(*vertex);
            }
        }
        LinearExpr sum;
        int cnt = 0;
        for (int j : undetermined_neighbours) {
            sum += decision_vars[translation_pace_to_ilp[j]];
            cnt++;
        }
        if (mds_context.is_undetermined(newToOldIndex[i])){
            sum += decision_vars[translation_pace_to_ilp[i]];
             cnt++;
        }
        if (cnt == 0){
            std::cout << mds_context.is_excluded(newToOldIndex[i]);
            std::cout << mds_context.is_removed(newToOldIndex[i]);
            std::cout << mds_context.is_undetermined(newToOldIndex[i]);
            std::cout << "is_bad" << std::endl;
            std::cout << num_neighbours << std::endl;
        }
        cp_model.AddGreaterOrEqual(sum , 1);
    }

    LinearExpr sum;
    for (auto && decision_var : decision_vars) {
        sum += decision_var;
    }
    //cp_model.AddGreaterOrEqual(700, sum);
    cp_model.Minimize(sum);
    // Solving part.
    SatParameters param;
    //
    param.set_num_search_workers(4);
    param.set_max_time_in_seconds(1800);
    param.set_log_search_progress(false);
    //param.add_extra_subsolvers("quick_restart");
    const CpSolverResponse response = SolveWithParameters(cp_model.Build(),param);
    if (response.status() == CpSolverStatus::OPTIMAL)
    {
        std::vector<int> solution;
        for (int c = 0; c < decision_vars.size(); c++) {
            if (1 == SolutionIntegerValue(response, decision_vars[c])) {
                solution.push_back(translation_ilp_to_pace[c]);
            }
        }
        return solution;
    }
        if (response.status() == CpSolverStatus::INFEASIBLE)
        {
            throw std::runtime_error("no optimal solution");
        }
        std::cout << response.status() << std::endl;
        std::vector<int> solution;
        std::ofstream outfile("/home/floris/github/minimum-dominating-set/score.txt", std::ios::app);
        if (!outfile) {
            std::cerr << "Error: Could not open file for writing.\n";
            return solution;
        }
        outfile << "timed_out" << "\n";
        std::cout << "time_out" << std::endl;
        Logger::timed_out = true;
        return solution;

}
