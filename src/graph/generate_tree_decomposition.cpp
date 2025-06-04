//
// Created by floris on 3/18/25.
//
#include <htd/main.hpp>
#include <csignal>
#include <memory>
#include <chrono>
#include "generate_tree_decomposition.h"
#include <queue>
#include <boost/graph/connected_components.hpp>
#include "../util/logger.h"

//Fitness function for the decomposition process.
class FitnessFunction_default : public htd::ITreeDecompositionFitnessFunction{
public:
    FitnessFunction_default(void)= default;

    ~FitnessFunction_default()= default;

    htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const {
        HTD_UNUSED(graph)
        //actual fitness function.
        return new htd::FitnessEvaluation(1,
                                          -(double)(decomposition.maximumBagSize()));
    }

    FitnessFunction_default * clone(void) const {
        return new FitnessFunction_default();
    }
};


class FitnessFunction_height : public htd::ITreeDecompositionFitnessFunction
{
    public:
        FitnessFunction_height(void)= default;

        ~FitnessFunction_height()=default;

        htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const {
            HTD_UNUSED(graph)
            //actual fitness function.
            return new htd::FitnessEvaluation(2,
                                              -(double)(decomposition.maximumBagSize()),
                                              -(double)(decomposition.height()));
        }

        FitnessFunction_height * clone(void) const {
            return new FitnessFunction_height();
        }
};

class FitnessFunction_minimize_join : public htd::ITreeDecompositionFitnessFunction
{
public:
    FitnessFunction_minimize_join(void)= default;

    ~FitnessFunction_minimize_join()=default;

    htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const {
        HTD_UNUSED(graph)
        //actual fitness function.
        double threshold = 10;
        double penalty = 0;
        double join_node_count = decomposition.joinNodeCount();
        auto join_nodes = decomposition.joinNodes();
        for (auto join_node : join_nodes) {
            std::size_t bag_size = decomposition.bagSize(join_node);
            if (bag_size > threshold) {
                double excess = static_cast<double>(bag_size - threshold);
                penalty += std::pow(excess, 2.5);
            }
        }


        return new htd::FitnessEvaluation(2,
                                          -(double)(decomposition.maximumBagSize()),
                                          -(double)(penalty));
    }

    FitnessFunction_minimize_join * clone(void) const {
        return new FitnessFunction_minimize_join();
    }
};


std::unique_ptr<NICE_TREE_DECOMPOSITION> generate_td(adjacencyListBoost& reduced_graph)
{
    std::cout << "Generating tree decomposition..." << std::endl;
    //Create a management instance of the 'htd' library in order to allow centralized configuration.
    const std::unique_ptr<htd::LibraryInstance> manager(htd::createManagementInstance(htd::Id::FIRST));
    std::unique_ptr<NICE_TREE_DECOMPOSITION> nice_tree_decomposition;
    std::srand(3);

    // Create a new graph instance which can handle (multi-)hyperedges.
    htd::IMutableMultiGraph * graph =
        manager->multiGraphFactory().createInstance();

    //load in reduced_graph.
    const uint num_graph = boost::num_vertices(reduced_graph);
    graph->addVertices(num_graph);
    int edge_counter = 0;

    // Add two edges to the graph.
    for (auto [itt_edge, itt_edge_end] = boost::edges(reduced_graph); itt_edge != itt_edge_end; ++itt_edge)
    {
        const uint target = boost::target(*itt_edge, reduced_graph) + 1;
        const uint source = boost::source(*itt_edge, reduced_graph) + 1;
        graph->addEdge(target, source);
        edge_counter++;
    }

    // Create an instance of the fitness function. (defined above)
    const FitnessFunction_minimize_join fitnessFunction;

    /**
     *  This operation changes the root of a given decomposition so that the provided
     *  fitness function is maximized. When no fitness function is provided to the
     *  constructor, the constructed optimization operation does not perform any
     *  optimization and only applies provided manipulations.
     */
    auto * operation = new htd::TreeDecompositionOptimizationOperation(manager.get());

    /**
     *  Set the previously created management instance to support graceful termination.
     */
    operation->setManagementInstance(manager.get());

    /**
     *  Set the vertex selections strategy (default = exhaustive).
     *
     *  In this case, we want to select (at most) 10 vertices of the input decomposition randomly.
     */
    operation->setVertexSelectionStrategy(new htd::RandomVertexSelectionStrategy(10));

    /**
     *  Set desired manipulations. In this case we want a nice (= normalized) tree decomposition.
     */
    operation->addManipulationOperation(new htd::NormalizationOperation(manager.get(), false, true, false, false));

    /**
     *  Optionally, we can set the vertex elimination algorithm.
     *  We decide to use the min-degree heuristic in this case.
     */

    manager->orderingAlgorithmFactory()
        .setConstructionTemplate(new htd::MinDegreeOrderingAlgorithm(manager.get()));

    // Get the default tree decomposition algorithm. One can also choose a custom one.
    htd::ITreeDecompositionAlgorithm * baseAlgorithm =
        manager->treeDecompositionAlgorithmFactory().createInstance();

    // htd::CombinedWidthMinimizingTreeDecompositionAlgorithm * algorithm_challenge = new htd::CombinedWidthMinimizingTreeDecompositionAlgorithm(manager.get());
    //
    // htd::AdaptiveWidthMinimizingTreeDecompositionAlgorithm * adaptiveAlgorithm = new htd::AdaptiveWidthMinimizingTreeDecompositionAlgorithm(manager.get());
    //
    // htd::BucketEliminationTreeDecompositionAlgorithm * algorithm1 = new htd::BucketEliminationTreeDecompositionAlgorithm(manager.get());
    //
    // algorithm1->setOrderingAlgorithm(new htd::MinDegreeOrderingAlgorithm(manager.get()));
    //
    // adaptiveAlgorithm->addDecompositionAlgorithm(algorithm1);
    //
    // htd::BucketEliminationTreeDecompositionAlgorithm * algorithm2 = new htd::BucketEliminationTreeDecompositionAlgorithm(manager.get());
    //
    // algorithm2->setOrderingAlgorithm(new htd::MinFillOrderingAlgorithm(manager.get()));
    //
    // adaptiveAlgorithm->addDecompositionAlgorithm(algorithm2);
    //
    // htd::BucketEliminationTreeDecompositionAlgorithm * algorithm3 = new htd::BucketEliminationTreeDecompositionAlgorithm(manager.get());
    //
    // algorithm3->setOrderingAlgorithm(new htd::MaximumCardinalitySearchOrderingAlgorithm(manager.get()));
    //
    // adaptiveAlgorithm->addDecompositionAlgorithm(algorithm3,
    //                                              [](const htd::IMultiHypergraph & graph, const htd::IPreprocessedGraph & preprocessedGraph)
    // {
    //     HTD_UNUSED(graph)
    //
    //     return preprocessedGraph.vertexCount() <= 10240;
    // });
    // std::size_t iterations = 10;
    // adaptiveAlgorithm->setIterationCount(iterations);
    // std::size_t patienceOption = 3;
    // adaptiveAlgorithm->setNonImprovementLimit(patienceOption);
    //
    // adaptiveAlgorithm->setDecisionRounds(5);
    //
    // algorithm_challenge->addDecompositionAlgorithm(new htd::TrivialTreeDecompositionAlgorithm(manager.get()));
    //
    // algorithm_challenge->addDecompositionAlgorithm(adaptiveAlgorithm);


    // htd::GraphPreprocessor * preprocessor = new htd::GraphPreprocessor(manager.get());
    // preprocessor->setPreprocessingStrategy(3);
    // preprocessor->setIterationCount(256);
    // preprocessor->setNonImprovementLimit(64);
    /**
     *  Create a new instance of htd::IterativeImprovementTreeDecompositionAlgorithm based
     *  on the base algorithm and the fitness function. Note that the fitness function can
     *  be an arbiraty one and can differ from the one used in the optimization operation.
     */
    htd::IterativeImprovementTreeDecompositionAlgorithm algorithm(manager.get(),
                                                                  baseAlgorithm,
                                                                  fitnessFunction.clone());

    /**
     *  Set the maximum number of iterations after which the best decomposition with
     *  respect to the fitness function shall be returned. Use value 1 to make the
     *  iterative algorithm return the first decomposition found.
     */
    algorithm.setIterationCount(10);

    /**
     *  Set the maximum number of iterations without improvement after which the algorithm returns
     *  best decomposition with respect to the fitness function found so far. A limit of 0 aborts
     *  the algorithm after the first non-improving solution has been found, i.e. the algorithm
     *  will perform a simple hill-climbing approach.
     */
    algorithm.setNonImprovementLimit(50);

    // Record the optimal maximal bag size of the tree decomposition to allow printing the progress.
    std::size_t optimalBagSize = (std::size_t)-1;

    /**
     *  Compute the decomposition. Note that the additional, optional parameter of the function
     *  computeDecomposition() in case of htd::IterativeImprovementTreeDecompositionAlgorithm
     *  can be used to intercept every new decomposition. In this case we output some
     *  intermediate information upon perceiving an improved decompostion.
     */
    htd::ITreeDecomposition * decomposition =
        algorithm.computeDecomposition(*graph, [&](const htd::IMultiHypergraph & graph,
                                                   const htd::ITreeDecomposition & decomposition,
                                                   const htd::FitnessEvaluation & fitness){});

    std::cout << "10 itterations is fast" << std::endl;
    std::cout << decomposition->maximumBagSize() << std::endl;
    std::cout << decomposition->exchangeNodeCount() << std::endl;
    // If a decomposition was found we want to print it to stdout.
    if (decomposition != nullptr) {
        //Check whether the algorithm indeed computed a valid decomposition.
        if (!manager->isTerminated() && algorithm.isSafelyInterruptible()) {
            // check it worth optimizing further. (if treewidth is smaller than 32).
            if (decomposition->maximumBagSize() < 11){
                algorithm.setIterationCount(10); // set iterations to infinite.

                /**
               *  Set the optimization operation as manipulation operation in order
               *  to choose the optimal root reducing height of the tree decomposition.
               */
                baseAlgorithm->addManipulationOperation(operation);

                //run where you left off.
                htd::ITreeDecomposition * decomposition = algorithm.computeDecomposition(*graph, [&](const htd::IMultiHypergraph & graph,
                                                   const htd::ITreeDecomposition & decomposition,
                                                   const htd::FitnessEvaluation & fitness){});
                //If further optimizations is done as well.
                if (decomposition != nullptr){
                    if (!manager->isTerminated() || algorithm.isSafelyInterruptible()){
                        std::cout << decomposition->maximumBagSize() << std::endl;
                        nice_tree_decomposition = std::make_unique<NICE_TREE_DECOMPOSITION>(reduced_graph, decomposition);
                        std::cout << "i want to read" << std::endl;
                    }
                }
            }
             else if (decomposition->maximumBagSize() < 50){
                 // Print the size of the largest bag of the decomposition to stdout.
                 algorithm.setIterationCount(0); // set iterations to infinite.
                 algorithm.setNonImprovementLimit(500);

                 /**
                *  Set the optimization operation as manipulation operation in order
                *  to choose the optimal root reducing height of the tree decomposition.
                */
                 baseAlgorithm->addManipulationOperation(operation);

                 //run where you left off.
                 htd::ITreeDecomposition * decomposition = algorithm.computeDecomposition(*graph, [&](const htd::IMultiHypergraph & graph,
                                                    const htd::ITreeDecomposition & decomposition,
                                                    const htd::FitnessEvaluation & fitness){});
                //If further optimizations is done as well.
                if (decomposition != nullptr){
                    if (!manager->isTerminated() || algorithm.isSafelyInterruptible()){
                        std::cout << "actual running treewidth: "<< decomposition->maximumBagSize() - 1 << std::endl;
                        nice_tree_decomposition = std::make_unique<NICE_TREE_DECOMPOSITION>(reduced_graph, decomposition);
                        std::cout << "read" << std::endl;
                    }
                }
            }else {
                std::cout << decomposition->maximumBagSize() << std::endl;
                Logger::treewidth.push_back(decomposition->maximumBagSize());
                if (Logger::maximum_treewidth < decomposition->maximumBagSize()){
                    Logger::maximum_treewidth = decomposition->maximumBagSize();
                }
            }
        }
        delete decomposition;
    }

    delete graph;

    //manager is a smart pointer so should go out of scope.
    return nice_tree_decomposition;
}

