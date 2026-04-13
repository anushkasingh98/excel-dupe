#import "GLGridView.h"
#import "GLCellEditorView.h"
#import "GLColumnHeaderView.h"
#import "GLRowHeaderView.h"
#import "../Toolbar/GLFormulaBarView.h"

/* Appearance constants */
static const CGFloat kGridLineWidth = 0.5;

@implementation GLGridView {
    BOOL _isEditing;
    NSTrackingArea *_trackingArea;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        _isEditing = NO;
        _cellEditor = [[GLCellEditorView alloc] initWithFrame:NSZeroRect];
        _cellEditor.hidden = YES;
        _cellEditor.gridView = self;
        [self addSubview:_cellEditor];
    }
    return self;
}

- (BOOL)isFlipped {
    return YES; /* Origin at top-left, like a spreadsheet */
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

#pragma mark - Coordinate Mapping

- (CGFloat)xForCol:(uint16_t)col {
    if (!_engine) return 0;
    SCSheet *sheet = sc_engine_active_sheet(_engine);
    CGFloat x = 0;
    for (uint16_t c = 0; c < col; c++) {
        x += sc_sheet_get_col_width(sheet, c);
    }
    return x;
}

- (CGFloat)yForRow:(uint32_t)row {
    if (!_engine) return 0;
    SCSheet *sheet = sc_engine_active_sheet(_engine);
    CGFloat y = 0;
    for (uint32_t r = 0; r < row; r++) {
        y += sc_sheet_get_row_height(sheet, r);
    }
    return y;
}

- (NSRect)rectForRow:(uint32_t)row col:(uint16_t)col {
    SCSheet *sheet = sc_engine_active_sheet(_engine);
    if (!sheet) return NSZeroRect;

    CGFloat x = [self xForCol:col];
    CGFloat y = [self yForRow:row];
    CGFloat w = sc_sheet_get_col_width(sheet, col);
    CGFloat h = sc_sheet_get_row_height(sheet, row);
    return NSMakeRect(x, y, w, h);
}

- (BOOL)cellAtPoint:(NSPoint)point row:(uint32_t *)outRow col:(uint16_t *)outCol {
    if (!_engine) return NO;
    SCSheet *sheet = sc_engine_active_sheet(_engine);

    /* Find column */
    CGFloat x = 0;
    uint16_t col = 0;
    for (; col < SC_MAX_COLS; col++) {
        CGFloat w = sc_sheet_get_col_width(sheet, col);
        if (point.x < x + w) break;
        x += w;
    }

    /* Find row */
    CGFloat y = 0;
    uint32_t row = 0;
    for (; row < SC_MAX_ROWS; row++) {
        CGFloat h = sc_sheet_get_row_height(sheet, row);
        if (point.y < y + h) break;
        y += h;
    }

    if (outRow) *outRow = row;
    if (outCol) *outCol = col;
    return YES;
}

#pragma mark - Drawing

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    if (!_engine) {
        [[NSColor windowBackgroundColor] setFill];
        NSRectFill(dirtyRect);
        return;
    }

    SCSheet *sheet = sc_engine_active_sheet(_engine);
    SCSelection *sel = _engine->selection;
    if (!sheet || !sel) return;

    /* Background */
    [[NSColor controlBackgroundColor] setFill];
    NSRectFill(dirtyRect);

    /* Determine visible rows/columns from dirty rect */
    uint32_t startRow = 0, endRow = 0;
    uint16_t startCol = 0, endCol = 0;

    CGFloat y = 0;
    BOOL foundStartRow = NO;
    for (uint32_t r = 0; r < SC_MAX_ROWS && y < NSMaxY(dirtyRect); r++) {
        CGFloat h = sc_sheet_get_row_height(sheet, r);
        if (!foundStartRow && y + h > NSMinY(dirtyRect)) {
            startRow = r;
            foundStartRow = YES;
        }
        if (y < NSMaxY(dirtyRect)) endRow = r;
        y += h;
    }

    CGFloat x = 0;
    BOOL foundStartCol = NO;
    for (uint16_t c = 0; c < SC_MAX_COLS && x < NSMaxX(dirtyRect); c++) {
        CGFloat w = sc_sheet_get_col_width(sheet, c);
        if (!foundStartCol && x + w > NSMinX(dirtyRect)) {
            startCol = c;
            foundStartCol = YES;
        }
        if (x < NSMaxX(dirtyRect)) endCol = c;
        x += w;
    }

    /* Draw gridlines */
    [[NSColor separatorColor] setStroke];
    NSBezierPath *gridPath = [NSBezierPath bezierPath];
    [gridPath setLineWidth:kGridLineWidth];

    /* Horizontal lines */
    y = [self yForRow:startRow];
    for (uint32_t r = startRow; r <= endRow + 1; r++) {
        [gridPath moveToPoint:NSMakePoint(NSMinX(dirtyRect), y)];
        [gridPath lineToPoint:NSMakePoint(NSMaxX(dirtyRect), y)];
        y += sc_sheet_get_row_height(sheet, r);
    }

    /* Vertical lines */
    x = [self xForCol:startCol];
    for (uint16_t c = startCol; c <= endCol + 1; c++) {
        [gridPath moveToPoint:NSMakePoint(x, NSMinY(dirtyRect))];
        [gridPath lineToPoint:NSMakePoint(x, NSMaxY(dirtyRect))];
        x += sc_sheet_get_col_width(sheet, c);
    }
    [gridPath stroke];

    /* Draw cell contents */
    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setLineBreakMode:NSLineBreakByClipping];

    NSDictionary *defaultAttrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:12],
        NSForegroundColorAttributeName: [NSColor textColor],
        NSParagraphStyleAttributeName: paraStyle
    };

    char displayBuf[256];

    for (uint32_t r = startRow; r <= endRow; r++) {
        for (uint16_t c = startCol; c <= endCol; c++) {
            NSRect cellRect = [self rectForRow:r col:c];

            /* Cell background for styled cells */
            SCCell *cell = sc_sheet_get_cell(sheet, r, c);
            if (cell && cell->style && cell->style->has_bg_color) {
                uint32_t bgc = cell->style->bg_color;
                CGFloat red   = ((bgc >> 16) & 0xFF) / 255.0;
                CGFloat green = ((bgc >> 8)  & 0xFF) / 255.0;
                CGFloat blue  = (bgc & 0xFF) / 255.0;
                [[NSColor colorWithRed:red green:green blue:blue alpha:1.0] setFill];
                NSRectFill(cellRect);
            }

            /* Selection highlight */
            if (sc_selection_contains(sel, r, c)) {
                [[NSColor selectedContentBackgroundColor] setFill];
                NSRectFillUsingOperation(cellRect, NSCompositingOperationSourceOver);

                if (r == sel->active_cell.row && c == sel->active_cell.col) {
                    /* Active cell: thicker border */
                    [[NSColor keyboardFocusIndicatorColor] setStroke];
                    NSBezierPath *activePath = [NSBezierPath bezierPathWithRect:
                        NSInsetRect(cellRect, 0.5, 0.5)];
                    [activePath setLineWidth:2.0];
                    [activePath stroke];
                }
            }

            /* Cell text */
            if (cell) {
                sc_cell_get_display_string(cell, displayBuf, sizeof(displayBuf));
                if (displayBuf[0] != '\0') {
                    NSString *text = [NSString stringWithUTF8String:displayBuf];
                    NSMutableDictionary *attrs = [defaultAttrs mutableCopy];

                    /* Apply cell style */
                    if (cell->style) {
                        NSFontDescriptorSymbolicTraits traits = 0;
                        if (cell->style->bold) traits |= NSFontDescriptorTraitBold;
                        if (cell->style->italic) traits |= NSFontDescriptorTraitItalic;

                        CGFloat fontSize = cell->style->font_size > 0 ?
                                           cell->style->font_size : 12;

                        if (traits) {
                            NSFontDescriptor *desc = [[NSFont systemFontOfSize:fontSize].fontDescriptor
                                fontDescriptorWithSymbolicTraits:traits];
                            NSFont *font = [NSFont fontWithDescriptor:desc size:fontSize];
                            if (font) attrs[NSFontAttributeName] = font;
                        } else {
                            attrs[NSFontAttributeName] = [NSFont systemFontOfSize:fontSize];
                        }

                        if (cell->style->font_color) {
                            uint32_t fc = cell->style->font_color;
                            CGFloat r = ((fc >> 16) & 0xFF) / 255.0;
                            CGFloat g = ((fc >> 8)  & 0xFF) / 255.0;
                            CGFloat b = (fc & 0xFF) / 255.0;
                            attrs[NSForegroundColorAttributeName] =
                                [NSColor colorWithRed:r green:g blue:b alpha:1.0];
                        }

                        /* Alignment */
                        NSMutableParagraphStyle *ps = [paraStyle mutableCopy];
                        switch (cell->style->h_align) {
                            case 1: [ps setAlignment:NSTextAlignmentLeft]; break;
                            case 2: [ps setAlignment:NSTextAlignmentCenter]; break;
                            case 3: [ps setAlignment:NSTextAlignmentRight]; break;
                            default:
                                /* Numbers right-aligned by default */
                                if (cell->type == SC_CELL_NUMBER ||
                                    cell->type == SC_CELL_FORMULA) {
                                    [ps setAlignment:NSTextAlignmentRight];
                                }
                                break;
                        }
                        attrs[NSParagraphStyleAttributeName] = ps;
                    } else if (cell->type == SC_CELL_NUMBER ||
                               cell->type == SC_CELL_FORMULA) {
                        NSMutableParagraphStyle *ps = [paraStyle mutableCopy];
                        [ps setAlignment:NSTextAlignmentRight];
                        attrs[NSParagraphStyleAttributeName] = ps;
                    }

                    /* Error cells in red */
                    if (cell->type == SC_CELL_ERROR) {
                        attrs[NSForegroundColorAttributeName] = [NSColor systemRedColor];
                    }

                    NSRect textRect = NSInsetRect(cellRect, 4, 2);
                    [text drawInRect:textRect withAttributes:attrs];
                }
            }
        }
    }
}

#pragma mark - Mouse Events

- (void)mouseDown:(NSEvent *)event {
    NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
    uint32_t row;
    uint16_t col;

    if ([self cellAtPoint:loc row:&row col:&col]) {
        if (_isEditing) {
            [self commitEditing];
        }

        if (event.modifierFlags & NSEventModifierFlagShift) {
            sc_selection_extend_to(_engine->selection, row, col);
        } else if (event.modifierFlags & NSEventModifierFlagCommand) {
            sc_selection_add_range(_engine->selection, row, col, row, col);
        } else {
            sc_selection_set_cell(_engine->selection, row, col);
        }

        [self setNeedsDisplay:YES];
        [_columnHeaderView setNeedsDisplay:YES];
        [_rowHeaderView setNeedsDisplay:YES];
        [_formulaBar updateFromGridView:self];
    }
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
    uint32_t row;
    uint16_t col;

    if ([self cellAtPoint:loc row:&row col:&col]) {
        sc_selection_extend_to(_engine->selection, row, col);
        [self setNeedsDisplay:YES];
        [_columnHeaderView setNeedsDisplay:YES];
        [_rowHeaderView setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)event {
    if (event.clickCount == 2) {
        [self beginEditingActiveCell];
    }
}

#pragma mark - Keyboard Events

- (void)keyDown:(NSEvent *)event {
    if (!_engine) return;

    NSString *chars = event.charactersIgnoringModifiers;
    if (chars.length == 0) return;

    unichar ch = [chars characterAtIndex:0];
    BOOL shift = (event.modifierFlags & NSEventModifierFlagShift) != 0;

    switch (ch) {
        case NSUpArrowFunctionKey:
            if (shift) {
                sc_selection_extend_to(_engine->selection,
                    _engine->selection->active_cell.row > 0 ?
                        _engine->selection->ranges[_engine->selection->range_count - 1].end_row - 1 :
                        0,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_col);
            } else {
                sc_selection_move(_engine->selection, -1, 0);
            }
            break;
        case NSDownArrowFunctionKey:
            if (shift) {
                sc_selection_extend_to(_engine->selection,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_row + 1,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_col);
            } else {
                sc_selection_move(_engine->selection, 1, 0);
            }
            break;
        case NSLeftArrowFunctionKey:
            if (shift) {
                sc_selection_extend_to(_engine->selection,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_row,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_col > 0 ?
                        _engine->selection->ranges[_engine->selection->range_count - 1].end_col - 1 :
                        0);
            } else {
                sc_selection_move(_engine->selection, 0, -1);
            }
            break;
        case NSRightArrowFunctionKey:
            if (shift) {
                sc_selection_extend_to(_engine->selection,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_row,
                    _engine->selection->ranges[_engine->selection->range_count - 1].end_col + 1);
            } else {
                sc_selection_move(_engine->selection, 0, 1);
            }
            break;
        case '\r': /* Enter */
            if (_isEditing) {
                [self commitEditing];
            }
            sc_selection_move(_engine->selection, shift ? -1 : 1, 0);
            break;
        case '\t': /* Tab */
            if (_isEditing) {
                [self commitEditing];
            }
            sc_selection_move(_engine->selection, 0, shift ? -1 : 1);
            break;
        case 0x1B: /* Escape */
            if (_isEditing) {
                [self cancelEditing];
            }
            break;
        case 0x7F: /* Delete */
        case NSDeleteFunctionKey:
            sc_engine_clear_selection(_engine);
            [[NSNotificationCenter defaultCenter]
                postNotificationName:@"GLDocumentDidChange" object:self.window.windowController.document];
            break;
        default:
            /* Printable character: start editing */
            if (ch >= 0x20 && ch < 0xF700) {
                [self beginEditingActiveCell];
                [_cellEditor insertText:chars replacementRange:NSMakeRange(0, 0)];
                return;
            }
            break;
    }

    /* Scroll to make active cell visible */
    NSRect cellRect = [self rectForRow:_engine->selection->active_cell.row
                                  col:_engine->selection->active_cell.col];
    [self scrollRectToVisible:cellRect];

    [self setNeedsDisplay:YES];
    [_columnHeaderView setNeedsDisplay:YES];
    [_rowHeaderView setNeedsDisplay:YES];
    [_formulaBar updateFromGridView:self];
}

#pragma mark - Cell Editing

- (void)beginEditingActiveCell {
    if (!_engine || _isEditing) return;

    SCSelection *sel = _engine->selection;
    NSRect cellRect = [self rectForRow:sel->active_cell.row col:sel->active_cell.col];

    _cellEditor.frame = cellRect;
    _cellEditor.hidden = NO;
    _isEditing = YES;
    sel->editing = YES;

    /* Pre-fill with existing value */
    SCCell *cell = sc_sheet_get_cell(sc_engine_active_sheet(_engine),
                                     sel->active_cell.row, sel->active_cell.col);
    if (cell) {
        if (cell->formula_text) {
            [_cellEditor setStringValue:[NSString stringWithUTF8String:cell->formula_text]];
        } else {
            char buf[256];
            sc_cell_get_display_string(cell, buf, sizeof(buf));
            [_cellEditor setStringValue:[NSString stringWithUTF8String:buf]];
        }
    } else {
        [_cellEditor setStringValue:@""];
    }

    [self.window makeFirstResponder:_cellEditor];
}

- (void)commitEditing {
    if (!_isEditing || !_engine) return;

    NSString *value = [_cellEditor stringValue];
    SCSelection *sel = _engine->selection;

    sc_engine_set_cell_value(_engine, sel->active_cell.row, sel->active_cell.col,
                             [value UTF8String]);

    _cellEditor.hidden = YES;
    _isEditing = NO;
    sel->editing = NO;

    [self.window makeFirstResponder:self];
    [self setNeedsDisplay:YES];
    [_formulaBar updateFromGridView:self];

    /* Notify document of change */
    [[NSNotificationCenter defaultCenter]
        postNotificationName:@"GLDocumentDidChange" object:self.window.windowController.document];
}

- (void)cancelEditing {
    if (!_isEditing) return;

    _cellEditor.hidden = YES;
    _isEditing = NO;
    _engine->selection->editing = NO;

    [self.window makeFirstResponder:self];
    [self setNeedsDisplay:YES];
}

@end
