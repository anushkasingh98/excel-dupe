#import "GLRowHeaderView.h"
#import "GLGridView.h"

@implementation GLRowHeaderView

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

    NSScrollView *scrollView = _gridView.enclosingScrollView;
    CGFloat scrollY = scrollView ? scrollView.contentView.bounds.origin.y : 0;

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

    CGFloat y = -scrollY;
    CGFloat viewWidth = NSWidth(self.bounds);

    for (uint32_t r = 0; r < SC_MAX_ROWS && y < NSMaxY(dirtyRect); r++) {
        CGFloat h = sc_sheet_get_row_height(sheet, r);

        if (y + h > NSMinY(dirtyRect)) {
            BOOL isSelected = (r >= sel->ranges[0].start_row && r <= sel->ranges[0].end_row) ||
                              r == sel->active_cell.row;

            if (isSelected) {
                [[NSColor selectedContentBackgroundColor] setFill];
                NSRectFillUsingOperation(NSMakeRect(0, y, viewWidth, h),
                                         NSCompositingOperationSourceOver);
            }

            NSString *label = [NSString stringWithFormat:@"%u", r + 1];
            NSRect textRect = NSMakeRect(2, y + 2, viewWidth - 4, h - 4);
            [label drawInRect:textRect withAttributes:isSelected ? selectedAttrs : attrs];

            /* Horizontal separator */
            NSBezierPath *line = [NSBezierPath bezierPath];
            [line moveToPoint:NSMakePoint(0, y + h)];
            [line lineToPoint:NSMakePoint(viewWidth, y + h)];
            [line setLineWidth:0.5];
            [line stroke];
        }

        y += h;
    }

    /* Right border */
    [[NSColor separatorColor] setStroke];
    NSBezierPath *right = [NSBezierPath bezierPath];
    [right moveToPoint:NSMakePoint(viewWidth - 0.5, 0)];
    [right lineToPoint:NSMakePoint(viewWidth - 0.5, NSHeight(self.bounds))];
    [right setLineWidth:0.5];
    [right stroke];
}

@end
