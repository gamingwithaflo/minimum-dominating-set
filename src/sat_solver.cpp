#include "sat_solver.h"

using namespace operations_research::sat;

void ds_sat_solver(MDS_CONTEXT& mds_context){
    CpModelBuilder cp_model;

            std::unordered_map<int, int> translation_pace_to_ilp;
            //create decision variables.

            //operations_research::Domain domain(0, 1);
            const int num_undetermined_vertices = mds_context.num_nodes - mds_context.cnt_rem;
            std::vector<BoolVar> decision_vars;
            decision_vars.reserve(num_undetermined_vertices);

            int index = 0;
            for (int i = 0; i < num_undetermined_vertices; i++) {
                if (mds_context.is_undetermined(i)){
                    decision_vars.push_back(cp_model.NewBoolVar().WithName(std::to_string(i)));
                    translation_pace_to_ilp[i] = index++;
                }
            }
            absl::Span<const BoolVar> span(decision_vars);

            //Create constraint.
            std::vector<absl::Span<const BoolVar>> storage;
            for (int i = 0; i < mds_context.num_nodes; i++) {
                if (mds_context.is_dominated(i) || mds_context.is_ignored(i)){
                    continue;
                }
                auto [neigh_itt, neigh_itt_end] = mds_context.get_neighborhood_itt(i);
                std::vector<BoolVar> undetermined_neighbours;
                undetermined_neighbours.reserve(std::distance(neigh_itt, neigh_itt_end) + 1);
                for (; neigh_itt != neigh_itt_end; neigh_itt++) {
                    if (mds_context.is_undetermined(*neigh_itt)){
                        undetermined_neighbours.push_back(decision_vars[translation_pace_to_ilp[*neigh_itt]]);
                    }
                }

                if (mds_context.is_excluded(i)){
                    absl::Span<const BoolVar> span_undetermined_neighbours(undetermined_neighbours);
                    storage.push_back(span_undetermined_neighbours);
                    cp_model.AddAtLeastOne(storage.back());
                } else {
                    undetermined_neighbours.push_back(decision_vars[translation_pace_to_ilp[i]]);
                    absl::Span<const BoolVar> span_undetermined_neighbours(undetermined_neighbours);
                    storage.push_back(span_undetermined_neighbours);
                    cp_model.AddAtLeastOne(storage.back());
                }

                cp_model.Minimize(LinearExpr::Sum(span));
            }
            // Solving part.
            const CpSolverResponse response = Solve(cp_model.Build());
            if (response.status() == CpSolverStatus::OPTIMAL || response.status() == CpSolverStatus::FEASIBLE) {
                // Get the value of x in the solution.
                std::cout << "Maximum of objective function: " << response.objective_value() + mds_context.cnt_sel << "\n";
      } else {
          std::cout << "sad no optimal solution";
      }
}
