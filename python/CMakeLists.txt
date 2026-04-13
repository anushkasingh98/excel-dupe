# Python integration bridge (Phase 6)
# Placeholder — will be populated when Python embedding is implemented

if(Python3_FOUND)
    add_library(gl_python STATIC
        src/sc_python_bridge.c
        src/sc_python_api.c
    )

    target_include_directories(gl_python PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/engine/include
    )

    target_link_libraries(gl_python
        sc_engine
        Python3::Python
    )
endif()
