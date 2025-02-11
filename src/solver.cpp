// Minimal example to call the GLOP solver.
#include<cassert>
#include "Highs.h"
#include "graph/context.h"


namespace operations_research {
    void solve_dominating_set(MDS_CONTEXT& mds_context) {
        //initialize the needed information.
        auto [undetermined,map_pace_to_ilp] = mds_context.get_undetermined_vertices();
        int num_vars = undetermined.size();
        int total_vertices = mds_context.get_total_vertices();
        int num_constraints = total_vertices - mds_context.cnt_dom;

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
        ds_model.lp_.offset_ = mds_context.cnt_sol; //temp to check if they overlap.

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
            vertex v = mds_context.get_vertex_from_index(i);
            if (mds_context.is_dominated(v)) {
                continue; //dominated vertices  do not need a contraint.
            }
            auto [neigh_v_itt, neigh_v_itt_end] = mds_context.get_neighborhood_itt(v);
            if (mds_context.is_undetermined(v)) {
                a.index_.push_back(map_pace_to_ilp[v]);
                a.value_.push_back(1);
                cnt++;
            }
            for (;neigh_v_itt < neigh_v_itt_end; ++neigh_v_itt) {
                if (mds_context.is_undetermined(*neigh_v_itt)) {
                    a.index_.push_back(map_pace_to_ilp[*neigh_v_itt]);
                    a.value_.push_back(1);
                    cnt++;
                }
                else {
                    continue; //These decision variables have no influence.
                }
            }
            HighsInt last = a.start_.back() + cnt;
            a.start_.push_back(last);
        }
        ds_model.lp_.a_matrix_ = a;

        //create a highs instance
        Highs highs;
        HighsStatus return_status;



        return_status = highs.passModel(ds_model);
        assert(return_status == HighsStatus::kOk);
        const HighsLp& lp = highs.getLp();
        return_status = highs.run();
        assert(return_status == HighsStatus::kOk);
        const HighsSolution& solution = highs.getSolution();
    }
}  