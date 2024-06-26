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
    virtual double get(double x) = 0;

protected:
    double use_f(int index, double x)
    {
        if (index >= size)
        {
            return x;
        }
        if (fs[index] == nullptr || *(fs[index]) == nullptr)
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
    double get(double x)
    {
        return a;
    }

private:
    float &a;
};
class Random : public F
{
public:
    Random() : F(std::vector<F **>({}))
    {
        random.setSeed(123);
    };
    double get(double x)
    {
        return random.nextDouble() * 2 - 1;
    }

private:
    juce::Random random;
};

class Sine : public F
{
public:
    Sine(std::vector<F **> _f) : F(_f) {}
    double get(double x) override
    {
        return (double)std::sin(use_f(0, x));
    }
};
class Square : public F
{
public:
    Square(std::vector<F **> _f) : F(_f) {}
    double get(double rad) override
    {
        double x = use_f(0, rad);
        x = std::fmod(x, 2.0f * juce::MathConstants<double>::pi);
        if (x > juce::MathConstants<double>::pi)
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
    double get(double rad) override
    {
        rad = use_f(0, rad);
        double x = std::fmod(rad, 2.0f * juce::MathConstants<double>::pi);
        if (x > juce::MathConstants<double>::pi)
            return juce::jmap(x, juce::MathConstants<double>::pi, 2.0f * juce::MathConstants<double>::pi, -1.0, 0.0);
        else
            return juce::jmap(x, 0.0, juce::MathConstants<double>::pi, 0.0, 1.0);
    }
};
class Triangle : public F
{
public:
    Triangle(std::vector<F **> _f) : F(_f) {}
    double get(double rad) override
    {
        rad = use_f(0, rad);
        rad = rad + juce::MathConstants<double>::pi / 2.0;

        double x = std::fmod(rad, 2.0f * juce::MathConstants<double>::pi) - juce::MathConstants<double>::pi;
        double res = (x < 0) ? juce::jmap(x,
                                          double(-juce::MathConstants<double>::pi),
                                          double(0),
                                          double(-1),
                                          double(1))
                             : juce::jmap(x,
                                          double(0),
                                          double(juce::MathConstants<double>::pi),
                                          double(1),
                                          double(-1));
        return res;
    }
};

class Line : public F
{
public:
    Line(float &_start, float &_end, std::vector<F **> _f) : F(_f), start(_start), end(_end)
    {
    }

    double get(double rad)
    {
        rad = use_f(0, rad);
        double x = std::fmod(rad, juce::MathConstants<double>::twoPi);
        return juce::jmap(x, 0.0, juce::MathConstants<double>::twoPi, (double)start, (double)end);
    }

private:
    float &start;
    float &end;
};

class Add : public F
{
public:
    Add(std::vector<F **> _f) : F(_f){};
    double get(double x) override
    {
        return use_f(0, x) + use_f(1, x);
    }
};
class Substract : public F
{
public:
    Substract(std::vector<F **> _f) : F(_f){};
    double get(double x) override
    {
        return use_f(0, x) - use_f(1, x);
    }
};
class Multiply : public F
{
public:
    Multiply(std::vector<F **> _f) : F(_f){};

    double get(double x) override
    {
        return use_f(0, x) * use_f(1, x);
    }
};
class Divide : public F
{
public:
    Divide(std::vector<F **> _f) : F(_f){};

    double get(double x) override
    {
        return use_f(0, x) / use_f(1, x);
    }
};

class Concatenate : public F
{
public:
    Concatenate(std::vector<F **> _f) : F(_f){};

    double get(double rad) override
    {
        double x = std::fmod(rad, 4.0f * juce::MathConstants<double>::pi);
        if (x < juce::MathConstants<double>::twoPi)
        {
            return use_f(0, x);
        }
        else
        {
            return use_f(1, x);
        }
    }

private:
};