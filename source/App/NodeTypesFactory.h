
#pragma once
#include "RecoverableNodeGraph.h"
#include "NodeTypes.h"

class NoundTypesFactory : public TypesRecoverFactory
{
public:
    NoundTypesFactory()
    {
        factories[NodeTypes::Output] = new NodeCreateCommand<OutputNode>;
        factories[NodeTypes::Reverb] = new NodeCreateCommand<ReverbNode>;
        factories[NodeTypes::Waveform] = new NodeCreateCommand<WaveformNode>;
        factories[NodeTypes::FileReader] = new NodeCreateCommand<FileReaderNode>;
        factories[NodeTypes::AudioMath] = new NodeCreateCommand<AudioMathNode>;
        factories[NodeTypes::NumberMath] = new NodeCreateCommand<NumberMathNode>;
        factories[NodeTypes::Concatenate] = new NodeCreateCommand<ConcatenateNode>;
        factories[NodeTypes::FunctionMath] = new NodeCreateCommand<FunctionMathNode>;
        factories[NodeTypes::Const] = new NodeCreateCommand<ConstFunctionNode>;
        factories[NodeTypes::Random] = new NodeCreateCommand<RandomNode>;
        factories[NodeTypes::Oscillator] = new NodeCreateCommand<OscillatorNode>;
        factories[NodeTypes::LineNode] = new NodeCreateCommand<LineNode>;
        factories[NodeTypes::ConcatenateFunction] = new NodeCreateCommand<ConcatenateFNode>;
        factories[NodeTypes::RepeatNode] = new NodeCreateCommand<RepeatNode>;
    }

    EditorNode *getNode(int type_id) override
    {
        return factories[(NodeTypes)type_id]->execute();
    }
    std::map<NodeTypes, AbstractNodeCreateCommand *> factories;

    ~NoundTypesFactory()
    {
        for (auto &[_, f] : factories)
        {
            delete f;
        }
        factories.clear();
    }
};