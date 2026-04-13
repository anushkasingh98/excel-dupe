#import <Cocoa/Cocoa.h>
#include "sc_engine.h"

@class GLColumnHeaderView;
@class GLRowHeaderView;
@class GLFormulaBarView;
@class GLCellEditorView;

@interface GLGridView : NSView

@property (nonatomic, assign) SCEngine *engine;
@property (nonatomic, weak) GLColumnHeaderView *columnHeaderView;
@property (nonatomic, weak) GLRowHeaderView *rowHeaderView;
@property (nonatomic, weak) GLFormulaBarView *formulaBar;
@property (nonatomic, strong) GLCellEditorView *cellEditor;

/* Current visible scroll offset (set by scroll view) */
@property (nonatomic, readonly) uint32_t firstVisibleRow;
@property (nonatomic, readonly) uint16_t firstVisibleCol;

/* Get pixel rect for a cell (relative to this view's coordinate system) */
- (NSRect)rectForRow:(uint32_t)row col:(uint16_t)col;

/* Begin editing the active cell */
- (void)beginEditingActiveCell;
- (void)commitEditing;
- (void)cancelEditing;

@end
