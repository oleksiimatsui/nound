
#pragma once
#include "vector"
#include "string"
#include <JuceHeader.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "GraphProvider.h"
#include "NodeTypes.h"

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
    ItemWithNode(std::string _name, std::vector<Item *> _childs, NodeFactory *_factory) : Item(_name, _childs)
    {
        factory = _factory;
    };
    void onClick() override
    {
        auto graph = GraphProvider::getGraph();
        graph->addNode(factory->create());
    }
    NodeFactory *factory;
};

class ItemsFactory
{
public:
    static std::vector<Item *> getItems()
    {
        std::vector<Item *> i;

        i.push_back(new ItemWithNode("Start", std::vector<Item *>(), new StartNodeFactory()));
        i.push_back(new ItemWithNode("Speaker", std::vector<Item *>(), new SpeakerNodeFactory()));
        i.push_back(new ItemWithNode("File Reader", std::vector<Item *>(), new FileReaderFactory()));

        return i;
    }
};

struct ItemComponent final : public TreeViewItem
{
    ItemComponent(Item *i)
        : model(i), mouse_over(false)
    {
        theme = App::ThemeProvider::getCurrentTheme();
        for (auto item : model->childs)
        {
            addSubItem(new ItemComponent(item));
        };
    }
    ~ItemComponent()
    {
    }
    bool mightContainSubItems() override
    {
        return getNumSubItems() > 0;
    }
    void paintItem(Graphics &g, int width, int height) override
    {
        //  g.fillAll(theme->backgroundColor);
        g.setColour(theme->textColor);
        g.drawText(model->name, 0, 0, width, height, Justification::left);
    }
    void itemClicked(const MouseEvent &) override
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

class DropdownComponent final : public Component
{
public:
    DropdownComponent()
    {
        graph = GraphProvider::getGraph();
        Item *item = new Item("Add node", ItemsFactory::getItems());
        root = new ItemComponent(item);
        tree.setRootItem(root);
        //  tree.setRootItemVisible(false);
        addAndMakeVisible(tree);
        setVisible(true);

        tree.addMouseListener(this, false);
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
