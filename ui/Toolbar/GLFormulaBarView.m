#import "GLFormulaBarView.h"
#import "../Grid/GLGridView.h"

static const CGFloat kNameBoxWidth = 80.0;
static const CGFloat kPadding = 4.0;

@implementation GLFormulaBarView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        /* Name box (cell reference like "A1") */
        NSRect nameFrame = NSMakeRect(kPadding, kPadding,
                                       kNameBoxWidth, NSHeight(frameRect) - 2 * kPadding);
        _nameBox = [[NSTextField alloc] initWithFrame:nameFrame];
        _nameBox.font = [NSFont monospacedSystemFontOfSize:11 weight:NSFontWeightMedium];
        _nameBox.alignment = NSTextAlignmentCenter;
        _nameBox.stringValue = @"A1";
        _nameBox.bordered = YES;
        _nameBox.bezeled = YES;
        _nameBox.bezelStyle = NSTextFieldRoundedBezel;
        _nameBox.drawsBackground = YES;
        _nameBox.editable = YES;
        [self addSubview:_nameBox];

        /* Separator */
        CGFloat sepX = kNameBoxWidth + 2 * kPadding;
        NSBox *sep = [[NSBox alloc] initWithFrame:NSMakeRect(sepX, 2, 1, NSHeight(frameRect) - 4)];
        sep.boxType = NSBoxSeparator;
        [self addSubview:sep];

        /* Formula text field */
        CGFloat formulaX = sepX + kPadding + 1;
        NSRect formulaFrame = NSMakeRect(formulaX, kPadding,
                                          NSWidth(frameRect) - formulaX - kPadding,
                                          NSHeight(frameRect) - 2 * kPadding);
        _formulaField = [[NSTextField alloc] initWithFrame:formulaFrame];
        _formulaField.font = [NSFont systemFontOfSize:12];
        _formulaField.placeholderString = @"Enter value or formula";
        _formulaField.bordered = NO;
        _formulaField.bezeled = NO;
        _formulaField.drawsBackground = NO;
        _formulaField.editable = YES;
        _formulaField.autoresizingMask = NSViewWidthSizable;
        [self addSubview:_formulaField];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    /* Bottom border */
    [[NSColor separatorColor] setStroke];
    NSBezierPath *line = [NSBezierPath bezierPath];
    [line moveToPoint:NSMakePoint(0, 0.5)];
    [line lineToPoint:NSMakePoint(NSWidth(self.bounds), 0.5)];
    [line setLineWidth:0.5];
    [line stroke];
}

- (void)updateFromGridView:(GLGridView *)gridView {
    if (!gridView || !gridView.engine) return;

    SCSelection *sel = gridView.engine->selection;
    SCSheet *sheet = sc_engine_active_sheet(gridView.engine);

    /* Update name box with cell reference */
    char refBuf[16];
    sc_cellkey_to_ref(sel->active_cell, refBuf, sizeof(refBuf));
    _nameBox.stringValue = [NSString stringWithUTF8String:refBuf];

    /* Update formula field */
    SCCell *cell = sc_sheet_get_cell(sheet, sel->active_cell.row, sel->active_cell.col);
    if (cell) {
        if (cell->formula_text) {
            _formulaField.stringValue = [NSString stringWithUTF8String:cell->formula_text];
        } else {
            char buf[256];
            sc_cell_get_display_string(cell, buf, sizeof(buf));
            _formulaField.stringValue = [NSString stringWithUTF8String:buf];
        }
    } else {
        _formulaField.stringValue = @"";
    }
}

@end
