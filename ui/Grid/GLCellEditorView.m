#import "GLCellEditorView.h"
#import "GLGridView.h"

@implementation GLCellEditorView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        self.bordered = YES;
        self.bezeled = YES;
        self.bezelStyle = NSTextFieldSquareBezel;
        self.font = [NSFont systemFontOfSize:12];
        self.focusRingType = NSFocusRingTypeNone;
        self.drawsBackground = YES;
        self.backgroundColor = [NSColor textBackgroundColor];
        self.delegate = self;
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)controlTextDidEndEditing:(NSNotification *)obj {
    NSNumber *movement = obj.userInfo[@"NSTextMovement"];
    if (movement) {
        switch (movement.integerValue) {
            case NSReturnTextMovement:
                [_gridView commitEditing];
                sc_selection_move(_gridView.engine->selection, 1, 0);
                [_gridView setNeedsDisplay:YES];
                break;
            case NSTabTextMovement:
                [_gridView commitEditing];
                sc_selection_move(_gridView.engine->selection, 0, 1);
                [_gridView setNeedsDisplay:YES];
                break;
            case NSBacktabTextMovement:
                [_gridView commitEditing];
                sc_selection_move(_gridView.engine->selection, 0, -1);
                [_gridView setNeedsDisplay:YES];
                break;
            default:
                [_gridView commitEditing];
                break;
        }
    }
}

- (void)cancelOperation:(id)sender {
    [_gridView cancelEditing];
}

@end
