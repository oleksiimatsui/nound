
#pragma once
#include "vector"
#include "string"
#include <JuceHeader.h>
#include "NodeTypes.h"
#include "AppTheme.h"
#include "NodeGraph.h"

struct Item
{
    Item(){

    };
    Item(std::string _name) : name(_name){};
    Item(std::string _name, std::vector<Item *> _childs) : name(_name), childs(_childs){};

    virtual void onClick() {

    };

    virtual ~Item()
    {
    }
    std::string name;
    std::vector<Item *> childs;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Item);
};

struct ItemWithNode : public Item
{
    ItemWithNode(const std::string &_name, const std::vector<Item *> &_childs, AbstractNodeFactory *_factory, Graph *g) : Item(_name, _childs)
    {
        factory = _factory;
        graph = g;
    };
    void onClick() override
    {
        graph->addNode(factory->create());
    }
    ~ItemWithNode() override
    {
        delete factory;
        factory = nullptr;
    }
    AbstractNodeFactory *factory;
    Graph *graph;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemWithNode);
};

class ItemsFactory
{
public:
    static std::vector<Item *> getItems(Graph *g)
    {
        std::vector<Item *> i;

        // i.push_back(new Item("Triggers", std::vector<Item *>({new ItemWithNode("Start", std::vector<Item *>(), new NodeFactory<StartNode>), new ItemWithNode("Wait", std::vector<Item *>(), new NodeFactory<WaitNode>)})));

        i.push_back(new ItemWithNode(NodeNames::OutputNode, std::vector<Item *>(), new NodeFactory<OutputNode>, g));
        i.push_back(new Item("Functions", std::vector<Item *>(
                                              {new ItemWithNode(NodeNames::WaveformNode, std::vector<Item *>(), new NodeFactory<WaveformNode>, g),
                                               new ItemWithNode(NodeNames::FunctionMathNode, std::vector<Item *>(), new NodeFactory<FunctionMathNode>, g),
                                               new ItemWithNode(NodeNames::ConstNode, std::vector<Item *>(), new NodeFactory<ConstFunctionNode>, g)})));
        i.push_back(new Item("Audio", std::vector<Item *>(
                                          {
                                              new ItemWithNode(NodeNames::FileReader, std::vector<Item *>(), new NodeFactory<FileReaderNode>, g),
                                              new ItemWithNode(NodeNames::RandomNode, std::vector<Item *>(), new NodeFactory<RandomNode>, g),
                                              new ItemWithNode(NodeNames::Oscillator, std::vector<Item *>(), new NodeFactory<OscillatorNode>, g),
                                              new ItemWithNode(NodeNames::ReverbNode, std::vector<Item *>(), new NodeFactory<ReverbNode>, g),
                                              new ItemWithNode(NodeNames::AudioMathNode, std::vector<Item *>(), new NodeFactory<AudioMathNode>, g),
                                              new ItemWithNode(NodeNames::Concatenate, std::vector<Item *>(), new NodeFactory<ConcatenateNode>, g),
                                          })));

        i.push_back(new ItemWithNode(NodeNames::NumberMathNode, std::vector<Item *>(), new NodeFactory<NumberMathNode>, g));

        return i;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemsFactory);
};

struct ItemComponent final : public juce::TreeViewItem
{
    ItemComponent(Item *i)
        : model(i), mouse_over(false)
    {
        theme = App::ThemeProvider::getCurrentTheme();
        for (auto item : model->childs)
        {
            auto item_component = new ItemComponent(item);
            addSubItem(item_component);
        };
    }
    ~ItemComponent()
    {
        delete model;
        model = nullptr;
    }
    bool mightContainSubItems() override
    {
        return getNumSubItems() > 0;
    }
    void paintItem(juce::Graphics &g, int width, int height) override
    {
        theme = App::ThemeProvider::getCurrentTheme();
        g.setColour((theme->textColor));
        g.drawText(model->name, 0, 0, width, height, juce::Justification::left);
    }
    void itemClicked(const juce::MouseEvent &) override
    {
        if (mightContainSubItems())
            return;
        model->onClick();
    }
    juce::String getAccessibilityName() override
    {
        return model->name;
    }
    bool mouse_over = false;

private:
    Item *model;
    App::Theme *theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemComponent);
};

class DropdownComponent final : public juce::Component
{
public:
    DropdownComponent(Graph *g)
    {
        graph = g;
        root = nullptr;
        Item *item = new Item("Nodes", ItemsFactory::getItems(g));
        root = new ItemComponent(item);
        tree.setRootItem(root);
        tree.setDefaultOpenness(true);
        addAndMakeVisible(tree);
    }

    void resized() override
    {
        tree.setBounds(getLocalBounds());
    }

    ~DropdownComponent()
    {
        tree.deleteRootItem();
        //     tree.setRootItem(nullptr);
    }

private:
    Graph *graph;
    juce::TreeView tree;
    ItemComponent *root;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DropdownComponent);
};
