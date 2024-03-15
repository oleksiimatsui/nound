cmake_minimum_required(VERSION 3.22)

project(GUI_APP_EXAMPLE VERSION 0.0.1)


add_subdirectory("C:/Users/Asus/JUCE/JUCE" JUCE)
#find_package(JUCE CONFIG REQUIRED)

# juce_set_vst2_sdk_path(...)

juce_add_gui_app(GuiAppExample
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "Gui App Example")     # The name of the final executable, which can differ from the target name

target_sources(GuiAppExample
    PRIVATE
        Main.cpp
        MainComponent.cpp)

target_compile_definitions(GuiAppExample
    PRIVATE
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:GuiAppExample,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:GuiAppExample,JUCE_VERSION>")

target_link_libraries(GuiAppExample
    PRIVATE
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags) 
