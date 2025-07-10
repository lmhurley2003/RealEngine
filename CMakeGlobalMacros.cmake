#TODO make some of these constexpr bools under Program.hpp parameters to make these program dependent instead of
#compile-time dependend ?
if(DEFINED SIMPLE_VERTEX AND SIMPLE_VERTEX)
    add_compile_definitions(SIMPLE_VERTEX)
endif()

if(DEFINED INDEX_32BIT AND INDEX_32BIT)
    add_compile_definitions(INDEX_32BIT)
endif()

if(DEFINED COMBINED_VERTEX_INDEX_BUFFER AND COMBINED_VERTEX_INDEX_BUFFER)
    add_compile_definitions(COMBINED_VERTEX_INDEX_BUFFER)
endif()

if(DEFINED DYNAMIC_RENDERING AND DYNAMIC_RENDERING)
    add_compile_definitions(DYNAMIC_RENDERING)
endif()