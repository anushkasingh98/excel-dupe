#import "GLColumnHeaderView.h"
#import "GLGridView.h"

@implementation GLColumnHeaderView

- (BOOL)isFlipped {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    /* Header background */
    [[NSColor controlBackgroundColor] setFill];
    NSRectFill(dirtyRect);

    if (!_gridView || !_gridView.engine) return;

    SCSheet *sheet = sc_engine_active_sheet(_gridView.engine);
    SCSelection *sel = _gridView.engine->selection;
    if (!sheet) return;

    /* Draw column headers aligned with grid scroll */
    NSScrollView *scrollView = _gridView.enclosingScrollView;
    CGFloat scrollX = scrollView ? scrollView.contentView.bounds.origin.x : 0;

    NSDictionary *attrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:11 weight:NSFontWeightMedium],
        NSForegroundColorAttributeName: [NSColor secondaryLabelColor],
        NSParagraphStyleAttributeName: ({
            NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
            [ps setAlignment:NSTextAlignmentCenter];
            ps;
        })
    };

    NSDictionary *selectedAttrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:11 weight:NSFontWeightBold],
        NSForegroundColorAttributeName: [NSColor labelColor],
        NSParagraphStyleAttributeName: attrs[NSParagraphStyleAttributeName]
    };

    [[NSColor separatorColor] setStroke];

    CGFloat x = -scrollX;
    char colBuf[8];

    for (uint16_t c = 0; c < SC_MAX_COLS && x < NSMaxX(dirtyRect); c++) {
        CGFloat w = sc_sheet_get_col_width(sheet, c);

        if (x + w > NSMinX(dirtyRect)) {
            /* Highlight if column is selected */
            BOOL isSelected = (c >= sel->ranges[0].start_col && c <= sel->ranges[0].end_col) ||
                              c == sel->active_cell.col;

            if (isSelected) {
                [[NSColor selectedContentBackgroundColor] setFill];
                NSRectFillUsingOperation(NSMakeRect(x, 0, w, NSHeight(self.bounds)),
                                         NSCompositingOperationSourceOver);
            }

            sc_col_to_letter(c, colBuf, sizeof(colBuf));
            NSString *label = [NSString stringWithUTF8String:colBuf];
            NSRect textRect = NSMakeRect(x, 4, w, NSHeight(self.bounds) - 4);
            [label drawInRect:textRect withAttributes:isSelected ? selectedAttrs : attrs];

            /* Vertical separator */
            NSBezierPath *line = [NSBezierPath bezierPath];
            [line moveToPoint:NSMakePoint(x + w, 0)];
            [line lineToPoint:NSMakePoint(x + w, NSHeight(self.bounds))];
            [line setLineWidth:0.5];
            [line stroke];
        }

        x += w;
    }

    /* Bottom border */
    [[NSColor separatorColor] setStroke];
    NSBezierPath *bottom = [NSBezierPath bezierPath];
    [bottom moveToPoint:NSMakePoint(0, NSHeight(self.bounds) - 0.5)];
    [bottom lineToPoint:NSMakePoint(NSWidth(self.bounds), NSHeight(self.bounds) - 0.5)];
    [bottom setLineWidth:0.5];
    [bottom stroke];
}

@end
