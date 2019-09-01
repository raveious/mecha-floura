
find_program(MICROXRCEDDSGEN microxrceddsgen)

if (NOT MICROXRCEDDSGEN)
    message(FATAL_ERROR "Failed to find microxrceddsgen.")
endif(NOT MICROXRCEDDSGEN)

file(GLOB foobar "idl/*.idl")

foreach(bar ${foobar})

    get_filename_component(idl_name ${bar} NAME_WE)

    set(c_file ${idl_name}.c)
    set(header_file ${idl_name}.h)

    # Yes, this is completely wrong. ESP-IDF runs these as scripts and it really wierd, but it works
    execute_process(
        INPUT_FILE ${bar}
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/${idl_name}
        COMMAND ${MICROXRCEDDSGEN} -d ${CMAKE_BINARY_DIR}/generated/${idl_name} -replace ${bar}
    )

    set(COMPONENT_ADD_INCLUDEDIRS "${COMPONENT_ADD_INCLUDEDIRS} ${CMAKE_BINARY_DIR}/generated/${idl_name}")
endforeach(bar)

file(GLOB_RECURSE COMPONENT_SRCS "${CMAKE_BINARY_DIR}/generated/*/*.c")

set(COMPONENT_PRIV_REQUIRES microcdr newlib)

register_component()
