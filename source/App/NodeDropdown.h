
#pragma once
#include "vector"
#include "string"
#include <JuceHeader.h>
#include "AppTheme.h"
#include "NodeGraph.h"
#include "GraphProvider.h"
#include "NodeTypes.h"

class AbstractNodeFactory
{
public:
    virtual EditorNode *create() = 0;
};

template <class T>
class NodeFactory : public AbstractNodeFactory
{
public:
    EditorNode *create() override
    {
        return new T;
    }
};

struct Item
{
    Item(){

    };
    Item(std::string _name) : name(_name){};
    Item(std::string _name, std::vector<Item *> _childs) : name(_name), childs(_childs){};
    std::string name;
    std::vector<Item *> childs;
    virtual void onClick() {

    };
};

struct ItemWithNode : public Item
{
    ItemWithNode(const std::string &_name, const std::vector<Item *> &_childs, AbstractNodeFactory *_factory) : Item(_name, _childs)
    {
        factory = _factory;
    };
    void onClick() override
    {
        auto graph = GraphProvider::getGraph();
        graph->addNode(factory->create());
    }
    AbstractNodeFactory *factory;
};

class ItemsFactory
{
public:
    static std::vector<Item *> getItems()
    {
        std::vector<Item *> i;

        // i.push_back(new Item("Triggers", std::vector<Item *>({new ItemWithNode("Start", std::vector<Item *>(), new NodeFactory<StartNode>), new ItemWithNode("Wait", std::vector<Item *>(), new NodeFactory<WaitNode>)})));

        i.push_back(new ItemWithNode(NodeNames::OutputNode, std::vector<Item *>(), new NodeFactory<OutputNode>));
        i.push_back(new Item("Functions", std::vector<Item *>(
                                              {new ItemWithNode(NodeNames::WaveformNode, std::vector<Item *>(), new NodeFactory<WaveformNode>)})));
        i.push_back(new Item("Audio", std::vector<Item *>(
                                          {new ItemWithNode(NodeNames::FileReader, std::vector<Item *>(), new NodeFactory<FileReader>),
                                           new ItemWithNode(NodeNames::RandomNode, std::vector<Item *>(), new NodeFactory<RandomNode>),
                                           new ItemWithNode(NodeNames::OscillatorNode, std::vector<Item *>(), new NodeFactory<OscillatorNode>),
                                           new ItemWithNode(NodeNames::ReverbNode, std::vector<Item *>(), new NodeFactory<ReverbNode>),
                                           new ItemWithNode(NodeNames::FMNode, std::vector<Item *>(), new NodeFactory<FMNode>),
                                           new ItemWithNode(NodeNames::AudioMathNode, std::vector<Item *>(), new NodeFactory<AudioMathNode>),
                                           new ItemWithNode(NodeNames::Concatenate, std::vector<Item *>(), new NodeFactory<Concatenate>)})));

        i.push_back(new ItemWithNode(NodeNames::NumberMathNode, std::vector<Item *>(), new NodeFactory<NumberMathNode>));

        return i;
    }
};

struct ItemComponent final : public juce::TreeViewItem
{
    ItemComponent(Item *i)
        : model(i), mouse_over(false)
    {

        for (auto item : model->childs)
        {
            auto item_component = new ItemComponent(item);
            addSubItem(item_component);
        };
    }
    ~ItemComponent()
    {
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
};

class DropdownComponent final : public juce::Component
{
public:
    DropdownComponent()
    {
        graph = GraphProvider::getGraph();
        Item *item = new Item("Nodes", ItemsFactory::getItems());
        root = new ItemComponent(item);
        tree.setRootItem(root);
        tree.setDefaultOpenness(true);

        auto theme = App::ThemeProvider::getCurrentTheme();
        //  tree.setColour(TreeView::selectedItemBackgroundColourId, theme->dropdownMouseoverBackgroundColor);
        //  tree.setRootItemVisible(false);
        addAndMakeVisible(tree);
        setVisible(true);
    }

    void resized() override
    {
        tree.setBounds(getLocalBounds());
    }

    ~DropdownComponent()
    {
        delete root;
        tree.deleteRootItem(); // this deletes the children too...
    }

private:
    Graph *graph;
    juce::TreeView tree;
    ItemComponent *root;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DropdownComponent);
};
