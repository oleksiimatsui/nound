cmake_minimum_required(VERSION 3.15)

project(NodeGraph VERSION 0.0.1)



add_library(NodeGraph STATIC)



target_include_directories(NodeGraph
        INTERFACE
        $<TARGET_PROPERTY:NodeGraph,INCLUDE_DIRECTORIES>)

target_sources(NodeGraph
        PRIVATE
        NodeGraph.cpp
)
set_target_properties(NodeGraph
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)