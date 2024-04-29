
#pragma once
#include "RecoverableNodeGraph.h"
#include "NodeTypes.h"

class NoundTypesFactory : public TypesRecoverFactory
{
public:
    NoundTypesFactory()
    {
        factories[NodeTypes::Output] = new NodeFactory<OutputNode>;
        factories[NodeTypes::Reverb] = new NodeFactory<ReverbNode>;
        factories[NodeTypes::Waveform] = new NodeFactory<WaveformNode>;
        factories[NodeTypes::FileReader] = new NodeFactory<FileReaderNode>;
        factories[NodeTypes::AudioMath] = new NodeFactory<AudioMathNode>;
        factories[NodeTypes::NumberMath] = new NodeFactory<NumberMathNode>;
        factories[NodeTypes::Concatenate] = new NodeFactory<ConcatenateNode>;
        factories[NodeTypes::FunctionMath] = new NodeFactory<FunctionMathNode>;
        factories[NodeTypes::Const] = new NodeFactory<ConstFunctionNode>;
        factories[NodeTypes::Random] = new NodeFactory<RandomNode>;
        factories[NodeTypes::Oscillator] = new NodeFactory<OscillatorNode>;
    }

    EditorNode *getNode(int type_id) override
    {
        return factories[(NodeTypes)type_id]->create();
    }
    std::map<NodeTypes, AbstractNodeFactory *> factories;

    ~NoundTypesFactory()
    {
        for (auto &[_, f] : factories)
        {
            delete f;
        }
        factories.clear();
    }
};