set(COMPONENT_ADD_INCLUDEDIRS microcdr/include microcdr/include/ucdr microcdr/include/ucdr/types)

file(GLOB_RECURSE COMPONENT_SRCS microcdr/src/c/*.c)

set(COMPONENT_REQUIRES lwip)

register_component()

