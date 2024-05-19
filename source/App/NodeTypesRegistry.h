#pragma once
#include "string"

enum class NodeTypes
{
    Output,
    FileReader,
    Reverb,
    Waveform,
    AudioMath,
    NumberMath,
    Concatenate,
    FunctionMath,
    Const,
    Random,
    Oscillator,
    NumberNode,
    LineNode,
    ConcatenateFunction,
    RepeatNode,
    TrimNode,
    ResamplingNode,
    FilterNode
};

struct NodeNames
{
    static const std::string OutputNode;
    static const std::string FileReader;
    static const std::string ReverbNode;
    static const std::string RandomNode;
    static const std::string WaveformNode;
    static const std::string Oscillator;
    static const std::string AudioMathNode;
    static const std::string NumberMathNode;
    static const std::string Concatenate;
    static const std::string FunctionMathNode;
    static const std::string ConstNode;
    static const std::string NumberNode;
    static const std::string LineNode;
    static const std::string ConcatenateFunction;
    static const std::string RepeatNode;
    static const std::string TrimNode;
    static const std::string ResamplingNode;
    static const std::string FilterNode;
};
const std::string NodeNames::OutputNode = "Output";
const std::string NodeNames::FileReader = "File Reader";
const std::string NodeNames::ReverbNode = "Reverb";
const std::string NodeNames::RandomNode = "Random";
const std::string NodeNames::WaveformNode = "Basic Waveform";
const std::string NodeNames::AudioMathNode = "Audio Math";
const std::string NodeNames::NumberMathNode = "Number Math";
const std::string NodeNames::Concatenate = "Concatenate";
const std::string NodeNames::FunctionMathNode = "Arithmetic";
const std::string NodeNames::ConstNode = "Const";
const std::string NodeNames::Oscillator = "Oscillator";
const std::string NodeNames::NumberNode = "Number";
const std::string NodeNames::LineNode = "Line";
const std::string NodeNames::ConcatenateFunction = "Function Concatenate";
const std::string NodeNames::RepeatNode = "Repeat";
const std::string NodeNames::TrimNode = "Trim";
const std::string NodeNames::ResamplingNode = "Resampling";
const std::string NodeNames::FilterNode = "Band Pass Filter";