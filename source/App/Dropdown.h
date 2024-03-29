
#pragma once
#include "vector"
#include "string"
#include <JuceHeader.h>
#include "Theme.h"

struct Item
{
    Item(std::string _name) : name(_name){

                              };
    Item(std::string _name, std::vector<Item> _childs) : name(_name), childs(_childs){

                                                                      };
    std::string name;
    std::vector<Item> childs;
};

class ItemsFactory
{
public:
    static std::vector<Item> getItems()
    {
        std::vector<Item> i;

        i.push_back(Item("Start"));
        i.push_back(Item("Speaker"));
        i.push_back(Item("File Reader"));

        return i;
    }
};

struct ItemComponent final : public TreeViewItem
{
    ItemComponent(Item i)
        : model(i)
    {
        theme = App::ThemeProvider::getCurrentTheme();
    }
    bool mightContainSubItems() override
    {
        return getNumSubItems() > 0;
    }
    void paintItem(Graphics &g, int width, int height) override
    {
        g.setColour(theme->textColor);
        g.drawText(model.name, 0, 0, width, height, Justification::left);
    }
    juce::String getAccessibilityName() override
    {
        return model.name;
    }
    const Item model;
    App::Theme *theme;
};

struct RootItem final : public juce::TreeViewItem
{
    RootItem()
    {
        auto items = ItemsFactory::getItems();
        for (int i = 0; i < items.size(); ++i)
            addSubItem(new ItemComponent(items[i]));
    }

    bool mightContainSubItems() override
    {
        return true;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RootItem)
};

class DropdownComponent final : public juce::Component
{
public:
    DropdownComponent()
    {
        tree.setRootItem(&root);
        tree.setRootItemVisible(false);
        addAndMakeVisible(tree);
        RootItem *rootItem = new RootItem();
        tree.setRootItem(rootItem);
        setVisible(true);
    }
    void resized() override
    {
        tree.setBounds(getLocalBounds());
    }

    ~DropdownComponent()
    {
        tree.deleteRootItem(); // this deletes the children too...
    }

private:
    juce::TreeView tree;
    RootItem root;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DropdownComponent);
};
