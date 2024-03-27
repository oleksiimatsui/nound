#pragma once
#include "NodeGraph.h"
#include <JuceHeader.h>
#include "vector"

class StartNode : public Node{
public:
    StartNode():Node(){
        header = "Start";
        Output output(0,"trigger", PinType::Control, this);
        outputs.push_back(output);
    };
};

class SpeakerNode : public Node{
public:
    SpeakerNode():Node(){
        header = "Speaker";
        Input input(0,"numbers", PinType::Signal, this);
        inputs.push_back(input);
    };
};

class FileReader : public Node{
public:
    FileReader():Node(){
        header = "File Reader";
        inputs.push_back(Input(0,"trigger", PinType::Control, this));
        outputs.push_back(Output(0,"numbers", PinType::Signal,this));
    };
};