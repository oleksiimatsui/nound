#pragma once
#include <JuceHeader.h>

class F1
{
public:
    F1()
    {
        f = nullptr;
    };
    F1(F1 *_f)
    {
        f = _f;
    }
    virtual float get(float x) = 0;

protected:
    float use_f(float x)
    {
        if (f == nullptr)
        {
            return x;
        }
        return f->get(x);
    }

private:
    F1 *f;
};

class F2
{
public:
    F2(F1 *_f, F1 *_g) : f(_f), g(_g)
    {
    }
    virtual float get(float x, float y) = 0;

protected:
    float use_f(float x)
    {
        if (f == nullptr)
        {
            return x;
        }
        return f->get(x);
    }
    float use_g(float x)
    {
        if (g == nullptr)
        {
            return x;
        }
        return g->get(x);
    }

private:
    F1 *f;
    F1 *g;
};

class Const : public F1
{
public:
    Const(float _a) : F1()
    {
        a = _a;
    };
    float get(float x)
    {
        return a;
    }

private:
    float a;
};

class Sine : public F1
{
public:
    Sine() : F1(){};
    float get(float x) override
    {
        return (float)std::sin(use_f(x));
    }
};
class Square : public F1
{
public:
    Square() : F1(){};
    float get(float rad) override
    {
        float x = use_f(rad);
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
class Sawtooth : public F1
{
public:
    Sawtooth() : F1(){};
    float get(float rad) override
    {
        rad = use_f(rad);
        float x = std::fmod(rad, 2.0f * juce::MathConstants<float>::pi);
        if (x > juce::MathConstants<float>::pi)
            return juce::jmap(x, juce::MathConstants<float>::pi, 2.0f * juce::MathConstants<float>::pi, -1.0f, 0.0f);
        else
            return juce::jmap(x, 0.0f, juce::MathConstants<float>::pi, 0.0f, 1.0f);
    }
};
class Triangle : public F1
{
public:
    Triangle() : F1(){};
    float get(float rad) override
    {
        rad = use_f(rad);
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

class Add : public F2
{
public:
    Add(F1 *f, F1 *g) : F2(f, g){};
    float get(float x, float y) override
    {
        return use_f(x) + use_g(y);
    }
};
class Multiply : public F2
{
public:
    Multiply(F1 *f, F1 *g) : F2(f, g){};

    float get(float x, float y) override
    {
        return use_f(x) * use_g(y);
    }
};
class Divide : public F2
{
public:
    Divide(F1 *f, F1 *g) : F2(f, g){};

    float get(float x, float y) override
    {
        return use_f(x) / use_g(y);
    }
};