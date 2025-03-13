// exact-dominating-set.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "graph/graph_io.h"
#include "graph/context.h"
#include "reduce.h"

int main(int argc, char* argv[]);

void reduction_info(std::string path);

void reduction(std::string path, std::string path_td);

void output_reduced_graph(std::string path);

// TODO: Reference additional headers your program requires here.
