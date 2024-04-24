#pragma once
#include <JuceHeader.h>

class F
{
public:
    F(std::vector<F **> _f)
    {
        fs = _f;
        size = fs.size();
    }
    virtual float get(float x) = 0;

protected:
    float use_f(int index, float x)
    {
        if (index >= size)
        {
            return x;
        }
        if (*fs[index] == nullptr)
            return x;
        return (*(fs[index]))->get(x);
    }

private:
    std::vector<F **> fs;
    int size;
};

class Const : public F
{
public:
    Const(float &_a) : F(std::vector<F **>({})), a(_a){

                                                 };
    float get(float x)
    {
        return a;
    }

private:
    float &a;
};

class Sine : public F
{
public:
    Sine(std::vector<F **> _f) : F(_f) {}
    float get(float x) override
    {
        return (float)std::sin(use_f(0, x));
    }
};
class Square : public F
{
public:
    Square(std::vector<F **> _f) : F(_f) {}
    float get(float rad) override
    {
        float x = use_f(0, rad);
        x = std::fmod(x, 2.0f * juce::MathConstants<float>::pi);
        if (x > juce::MathConstants<float>::pi)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
};
class Sawtooth : public F
{
public:
    Sawtooth(std::vector<F **> _f) : F(_f) {}
    float get(float rad) override
    {
        rad = use_f(0, rad);
        float x = std::fmod(rad, 2.0f * juce::MathConstants<float>::pi);
        if (x > juce::MathConstants<float>::pi)
            return juce::jmap(x, juce::MathConstants<float>::pi, 2.0f * juce::MathConstants<float>::pi, -1.0f, 0.0f);
        else
            return juce::jmap(x, 0.0f, juce::MathConstants<float>::pi, 0.0f, 1.0f);
    }
};
class Triangle : public F
{
public:
    Triangle(std::vector<F **> _f) : F(_f) {}
    float get(float rad) override
    {
        rad = use_f(0, rad);
        float x = std::fmod(rad, 2.0f * juce::MathConstants<float>::pi);
        return (x >= juce::MathConstants<float>::twoPi) ? juce::jmap(x,
                                                                     (juce::MathConstants<float>::pi),
                                                                     (juce::MathConstants<float>::twoPi),
                                                                     (-1.0f),
                                                                     (1.0f))
                                                        : juce::jmap(x,
                                                                     (0.0f),
                                                                     (juce::MathConstants<float>::pi),
                                                                     (1.0f),
                                                                     (-1.0f));
    }
};

class Add : public F
{
public:
    Add(std::vector<F **> _f) : F(_f){};
    float get(float x) override
    {
        return use_f(0, x) + use_f(1, x);
    }
};
class Substract : public F
{
public:
    Substract(std::vector<F **> _f) : F(_f){};
    float get(float x) override
    {
        return use_f(0, x) - use_f(1, x);
    }
};
class Multiply : public F
{
public:
    Multiply(std::vector<F **> _f) : F(_f){};

    float get(float x) override
    {
        return use_f(0, x) * use_f(1, x);
    }
};
class Divide : public F
{
public:
    Divide(std::vector<F **> _f) : F(_f){};

    float get(float x) override
    {
        return use_f(0, x) / use_f(1, x);
    }
};
