#pragma once


namespace operations_research {

    std::vector<int> ilp_solver(MDS_CONTEXT& mds_context, adjacencyListBoost& graph, std::unordered_map<int,int>& newToOldIndex);

    double roundToInteger(double value);
}
