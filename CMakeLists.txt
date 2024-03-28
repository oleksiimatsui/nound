
cmake_minimum_required(VERSION 3.15)

project(Nound VERSION 0.0.1) 

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


add_subdirectory(source)