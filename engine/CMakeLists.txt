add_library(sc_engine STATIC
    src/sc_hashmap.c
    src/sc_cell.c
    src/sc_sheet.c
    src/sc_workbook.c
    src/sc_selection.c
    src/sc_undo.c
    src/sc_engine.c
)

target_include_directories(sc_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link math library
target_link_libraries(sc_engine PUBLIC m)
