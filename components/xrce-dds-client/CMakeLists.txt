set(COMPONENT_ADD_INCLUDEDIRS xrce-dds-client/include include)

file(GLOB_RECURSE COMPONENT_SRCS xrce-dds-client/src/c/*.c)

set(COMPONENT_REQUIRES lwip microcdr)

register_component()

