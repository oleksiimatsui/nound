#pragma once
#include "NodeGraph.h"

class GraphProvider
{
public:
    static void reset()
    {
        graph.reset(new Graph);
    };
    static Graph *getGraph()
    {
        if (graph.get() == nullptr)
        {
            throw;
        }
        return graph.get();
    };

private:
    static std::unique_ptr<Graph> graph;
};

std::unique_ptr<Graph> GraphProvider::graph = nullptr;