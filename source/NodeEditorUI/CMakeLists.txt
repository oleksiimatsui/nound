cmake_minimum_required(VERSION 3.15)

set     (TargetName   NodeEditorUI)

project(${TargetName} VERSION 0.0.1)

add_library(${TargetName} STATIC)

target_include_directories(${TargetName}
        INTERFACE
        $<TARGET_PROPERTY:${TargetName},INCLUDE_DIRECTORIES>)

target_include_directories(${TargetName}
        PUBLIC
        ../NodeGraph
)

target_sources(${TargetName}
        PRIVATE
        ConnectionComponent.h
        NodeComponent.h
        NodeEditorComponent.h
        PinComponent.h
        EditorNode.h
        RecoveringNodeGraph.h
)


target_compile_definitions(${TargetName}
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING= "$<TARGET_PROPERTY:${TargetName},JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING= "$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>")

target_link_libraries(${TargetName}
        PRIVATE
        NodeGraph
        juce::juce_gui_extra
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)