cmake_minimum_required(VERSION 3.15)

project(App VERSION 0.0.1)

juce_add_gui_app(App PRODUCT_NAME "App")    

juce_generate_juce_header(App)

target_include_directories(App
        PUBLIC
        ../NodeGraph
        ../NodeEditorUI
)

target_sources(App
    PRIVATE
        Main.cpp
        MainComponent.h
        NodeTypes.h 
)

target_compile_definitions(App
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:App,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:App,JUCE_VERSION>")

target_link_libraries(App
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        NodeEditorUI
        juce::juce_gui_extra
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)