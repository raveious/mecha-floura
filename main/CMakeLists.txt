# Edit following two lines to set component requirements (see docs)
#set(COMPONENT_REQUIRES rce-dds-client microcdr newlib)
#set(COMPONENT_PRIV_REQUIRES ) 

file(GLOB_RECURSE COMPONENT_SRCS "*.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
