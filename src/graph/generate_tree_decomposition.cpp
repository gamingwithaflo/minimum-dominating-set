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

//Fitness function for the decomposition process.
class FitnessFunction : public htd::ITreeDecompositionFitnessFunction
{
    public:
        FitnessFunction(void)= default;

        ~FitnessFunction()=default;

        htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const {
            HTD_UNUSED(graph)
            //actual fitness function.
            return new htd::FitnessEvaluation(2,
                                              -(double)(decomposition.maximumBagSize()),
                                              -(double)(decomposition.height()));
        }

        FitnessFunction * clone(void) const {
            return new FitnessFunction();
        }
};

std::unique_ptr<NICE_TREE_DECOMPOSITION> generate_td(adjacencyListBoost& reduced_graph)
{
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
    const FitnessFunction fitnessFunction;

    /**
     *  This operation changes the root of a given decomposition so that the provided
     *  fitness function is maximized. When no fitness function is provided to the
     *  constructor, the constructed optimization operation does not perform any
     *  optimization and only applies provided manipulations.
     */
    auto * operation = new htd::TreeDecompositionOptimizationOperation(manager.get(), fitnessFunction.clone());

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

    /**
     *  Set the optimization operation as manipulation operation in order
     *  to choose the optimal root reducing height of the tree decomposition.
     */
    baseAlgorithm->addManipulationOperation(operation);

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
    algorithm.setNonImprovementLimit(500);

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

    // If a decomposition was found we want to print it to stdout.
    if (decomposition != nullptr) {
        //Check whether the algorithm indeed computed a valid decomposition.
        if (!manager->isTerminated() && algorithm.isSafelyInterruptible()) {
            // check it worth optimizing further. (if treewidth is smaller than 32).
            if (decomposition->maximumBagSize() < 8) {
                std::cout << decomposition->maximumBagSize() << std::endl;
                nice_tree_decomposition = std::make_unique<NICE_TREE_DECOMPOSITION>(reduced_graph, decomposition);
                std::cout << "i want to read" << std::endl;
            }
             else if (decomposition->maximumBagSize() < 50){
                 // Print the size of the largest bag of the decomposition to stdout.
                 algorithm.setIterationCount(0); // set iterations to infinite.

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
        }
        delete decomposition;
    }

    delete graph;

    //manager is a smart pointer so should go out of scope.
    return nice_tree_decomposition;
}

