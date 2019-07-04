# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES xrce-dds-client microcdr newlib nvs_flash)
#set(COMPONENT_PRIV_REQUIRES )

file(GLOB_RECURSE COMPONENT_SRCS "*.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

find_file(IDL_SOURCE weather.idl ${CMAKE_SOURCE_DIR}/idl/)

#microxrceddsgen("${IDL_SOURCE}" COMPONENT_SRCS)

register_component()
