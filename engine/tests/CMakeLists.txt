# Simple test runner — each test is a standalone executable using assert()
# No external test framework dependency for now (cmocka can be added later via submodule)

add_executable(test_hashmap test_hashmap.c)
target_link_libraries(test_hashmap sc_engine)
add_test(NAME hashmap COMMAND test_hashmap)

add_executable(test_cell test_cell.c)
target_link_libraries(test_cell sc_engine)
add_test(NAME cell COMMAND test_cell)

add_executable(test_sheet test_sheet.c)
target_link_libraries(test_sheet sc_engine)
add_test(NAME sheet COMMAND test_sheet)

add_executable(test_undo test_undo.c)
target_link_libraries(test_undo sc_engine)
add_test(NAME undo COMMAND test_undo)

add_executable(test_selection test_selection.c)
target_link_libraries(test_selection sc_engine)
add_test(NAME selection COMMAND test_selection)
