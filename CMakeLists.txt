cmake_minimum_required(VERSION 3.22)


#########################################################################
project (NOUND VERSION 0.0.1)
set     (TargetName   NOUND)
set     (ProductName "NOUND")
#########################################################################

#get juce
add_subdirectory("C:/Users/Asus/JUCE/JUCE" JUCE) 

# Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
# Minimum MacOS target, set globally
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum OS X deployment version" FORCE)
# Universal Binary ?
option(UniversalBinary "Build universal binary for mac" ON)
if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
    set(MACOSX_BUNDLE TRUE)
endif()
# static linking in Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

juce_add_gui_app(${TargetName}
    PRODUCT_NAME ${ProductName})

juce_generate_juce_header(${TargetName})

target_sources(${TargetName}
    PRIVATE
        source/Main.cpp
        )

target_compile_definitions(${TargetName}
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>")

target_link_libraries(${TargetName}
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_utils
        juce::juce_graphics
        juce::juce_gui_basics

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags) 

