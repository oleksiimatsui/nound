
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
        i.push_back(new Item("Audio Input", std::vector<Item *>(
                                                {new ItemWithNode("File Reader", std::vector<Item *>(), new NodeFactory<FileReader>),
                                                 new ItemWithNode("Random", std::vector<Item *>(), new NodeFactory<RandomNode>),
                                                 new ItemWithNode("Sine", std::vector<Item *>(), new NodeFactory<SineNode>)})));
        i.push_back(new Item("Audio Effects", std::vector<Item *>({new ItemWithNode("Reverb", std::vector<Item *>(), new NodeFactory<ReverbNode>)})));
        i.push_back(new ItemWithNode("Speaker", std::vector<Item *>(), new NodeFactory<OutputNode>));
        i.push_back(new ItemWithNode("Audio Math", std::vector<Item *>(), new NodeFactory<AudioMathNode>));
        i.push_back(new ItemWithNode("Number Math", std::vector<Item *>(), new NodeFactory<NumberMathNode>));

        i.push_back(new ItemWithNode("Amplitude Trigger", std::vector<Item *>(), new NodeFactory<AudioToNumberNode>));

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
