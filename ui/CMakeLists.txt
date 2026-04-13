# GridLiberty macOS Application
# Only builds on macOS (guarded by parent CMakeLists.txt)

set(UI_SOURCES
    main.m
    GLAppDelegate.m
    GLDocument.m
    GLWindowController.m
    Grid/GLGridView.m
    Grid/GLCellEditorView.m
    Grid/GLColumnHeaderView.m
    Grid/GLRowHeaderView.m
    Toolbar/GLFormulaBarView.m
    Sidebar/GLSheetTabBar.m
)

add_executable(GridLiberty MACOSX_BUNDLE ${UI_SOURCES})

target_include_directories(GridLiberty PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/include
)

target_link_libraries(GridLiberty
    sc_engine
    ${COCOA_FW}
    ${QUARTZ_FW}
)

set_target_properties(GridLiberty PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    MACOSX_BUNDLE_BUNDLE_NAME "GridLiberty"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.gridliberty.app"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
)
