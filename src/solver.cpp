// Minimal example to call the GLOP solver.
#include<cassert>
#include "Highs.h"
#include "graph/context.h"


namespace operations_research {
    void solve_dominating_set(MDS_CONTEXT& mds_context) {
        //Create model
        vector<int>undetermined = mds_context.get_undetermined_vertices();


        //create a highs instance
        Highs highs;
        HighsStatus return_status;
    }
}  