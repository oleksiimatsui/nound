#pragma once
#include "NodeGraph.h"
#include <memory>
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
            graph.reset(new Graph);
        }
        return graph.get();
    };

private:
    static std::unique_ptr<Graph> graph;
};
