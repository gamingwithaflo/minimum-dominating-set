// Minimal example to call the GLOP solver.
#include<cassert>
#include "Highs.h"
#include "graph/context.h"
#include <stdexcept>
#include "util/logger.h"


namespace operations_research {
    double roundToInteger(double value) {
        const double tolerance = 1e-6;
        if (std::abs(value - 1.0) < tolerance) {
            return 1;
        }
        if (std::abs(value - 0.0) < tolerance) {
            return 0;
        }
        return value;  // Otherwise, return the original value
    }

    std::vector<int> ilp_solver(MDS_CONTEXT& mds_context, adjacencyListBoost& graph, std::unordered_map<int,int>& newToOldIndex) {
        //initialize the needed information.
        std::unordered_map<int,int> translation_pace_to_ilp;
        std::unordered_map<int,int> translation_ilp_to_pace;

        std::vector<int> undetermined_vertices;

        int index = 0;
        for (int i = 0; i < boost::num_vertices(graph); i++) {
            if (mds_context.is_undetermined(newToOldIndex[i])) {
                undetermined_vertices.push_back(i);
                translation_pace_to_ilp[i] = index;
                translation_ilp_to_pace[index] = i;
                index++;
            }
        }

        if (undetermined_vertices.empty()) {
            return std::vector<int>();
        }

        int num_vars = undetermined_vertices.size();
        int total_vertices = boost::num_vertices(graph);
        int counter_ign_dom = 0;
        //number of vertices you can ignore.
        for (int i = 0; i < total_vertices; ++i) {
            if (mds_context.is_dominated(newToOldIndex[i]) || mds_context.is_ignored(newToOldIndex[i])) {
                counter_ign_dom++;
            }
        }

        int num_constraints = total_vertices - counter_ign_dom;

        //Create model
        HighsModel ds_model;
        ds_model.lp_.num_col_ = num_vars; //number of decision variables.
        ds_model.lp_.num_row_ = num_constraints; //number of constraints.
        ds_model.lp_.sense_ = ObjSense::kMinimize;

        ds_model.lp_.col_cost_ = vector<double>(num_vars, 1); //taking a vertex has cost 1.
        ds_model.lp_.col_lower_ = vector<double>(num_vars, 0); //Decision variables >= 0.
        ds_model.lp_.col_upper_ = vector<double>(num_vars, 1); //Decision variables <= 1.
        ds_model.lp_.row_lower_ = vector<double>(num_constraints, 1);
        ds_model.lp_.row_upper_ = vector<double>(num_constraints, kHighsInf);
        //ds_model.lp_.offset_ = mds_context.cnt_sel; //temp to check if they overlap.

        //integrality
        ds_model.lp_.integrality_.resize(num_vars);
        for (int col = 0; col < num_vars; col++) {
            ds_model.lp_.integrality_[col] = HighsVarType::kInteger;
        }


        //define the constraints. (non-dominated vertices, either need to be 1 (if they are undetermined), a neighbor needs to be 1. 
        HighsSparseMatrix a;
        a.format_ = MatrixFormat::kRowwise;
        a.start_ = vector<HighsInt>(1, 0); // single element 0.
        for (int i = 0; i < total_vertices; i++) {
            HighsInt cnt = 0;
            //vertex v = mds_context.get_vertex_from_index(i);
            if (mds_context.is_dominated(newToOldIndex[i])) { //should be faster right?
                continue; //dominated vertices  do not need a contraint.
            }
            auto [neigh_v_itt, neigh_v_itt_end] = boost::adjacent_vertices(i, graph);
            //std::vector<int>index;
            if (mds_context.is_undetermined(newToOldIndex[i])) {
                //.push_back(translation_pace_to_ilp[i]);
                a.index_.push_back(translation_pace_to_ilp[i]);
                a.value_.push_back(1);
                cnt++;
            }

            for (;neigh_v_itt < neigh_v_itt_end; ++neigh_v_itt) {
                if (mds_context.is_undetermined(newToOldIndex[*neigh_v_itt])) {
                    //index.push_back(translation_pace_to_ilp[*neigh_v_itt]);
                    a.index_.push_back(translation_pace_to_ilp[*neigh_v_itt]);
                    a.value_.push_back(1);
                    cnt++;
                }
                else {
                    continue; //These decision variables have no influence.
                }
            }
            if (cnt == 0) {
                std::cout << "is_bad" << std::endl;
            }
            HighsInt last = a.start_.back() + cnt;
            a.start_.push_back(last);
        }
        ds_model.lp_.a_matrix_ = a;

        //create a highs instance
        Highs highs;
        HighsStatus return_status;
        double time_limit = 30 * 60; //time_limit in seconds.
        highs.setOptionValue("time_limit", time_limit);

        return_status = highs.passModel(ds_model);
        assert(return_status == HighsStatus::kOk);
        //const HighsLp& lp = highs.getLp();

        return_status = highs.run();
        if (return_status == HighsStatus::kOk) {
            const HighsModelStatus& model_status = highs.getModelStatus();
            //assert(model_status == HighsModelStatus::kOptimal);
            const HighsSolution& solution = highs.getSolution();
            vector<int>selected_vertices;
            for (auto i = 0; i < num_vars; ++i) {
                if (roundToInteger(solution.col_value[i]) == 1) { // is needed as there could be a very small int violation.
                    selected_vertices.push_back(translation_ilp_to_pace[i]);
                }
            }

            return selected_vertices;
        }
        if (return_status == HighsStatus::kWarning) {
            //time limit reached.
            Logger::timed_out = true;
            return vector<int>();
        }
        //should never happen (set a breakpoint for sure);
        throw new std::runtime_error("solver error");
    }
}  