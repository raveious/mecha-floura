cmake_minimum_required(VERSION 3.5)

# Adding the IDL generator modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/idl/cmake/modules")

# Adding some extra components
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components")

include(IDLGeneration)

# Doing this later because they define a macro that replaces the `project` keyword...
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mecha-floura)

