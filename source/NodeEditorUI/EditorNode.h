#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"

class EditorNode : public Node
{
public:
    EditorNode() : Node(){

                   };

    virtual juce::Component *getInternal() = 0;
};
