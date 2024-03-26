#pragma once
#include "NodeGraph.h"
#include <JuceHeader.h>
#include "vector"

class StartNode : public Node{
public:
    StartNode():Node(){
        header = "Start";
        Output output ={"trigger", PinType::Control};
        outputs.push_back(output);
    };
};

class SpeakerNode : public Node{
public:
    SpeakerNode():Node(){
        header = "Speaker";
        Input input ={"numbers", PinType::Signal};
        inputs.push_back(input);
    };
};

class FileReader : public Node{
public:
    FileReader():Node(){
        header = "File Reader";
        inputs.push_back({"trigger", PinType::Control});
        outputs.push_back({"numbers", PinType::Signal});
    };
};