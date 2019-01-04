configure_file(xrce-dds-client/include/uxr/client/config.h.in
    ${PROJECT_BINARY_DIR}/include/uxr/client/config.h
    )

set(COMPONENT_ADD_INCLUDEDIRS xrce-dds-client/include)

file(GLOB_RECURSE COMPONENT_SRCS xrce-dds-client/src/c/*.c)

set(COMPONENT_REQUIRES lwip microcdr)

register_component()

