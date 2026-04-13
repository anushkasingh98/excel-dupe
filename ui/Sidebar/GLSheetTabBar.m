#import "GLSheetTabBar.h"

static const CGFloat kTabWidth = 100.0;
static const CGFloat kTabHeight = 24.0;
static const CGFloat kAddButtonWidth = 28.0;
static const CGFloat kLeftPadding = 8.0;

@implementation GLSheetTabBar

- (BOOL)isFlipped {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    /* Background */
    [[NSColor windowBackgroundColor] setFill];
    NSRectFill(dirtyRect);

    /* Top border */
    [[NSColor separatorColor] setStroke];
    NSBezierPath *topLine = [NSBezierPath bezierPath];
    [topLine moveToPoint:NSMakePoint(0, 0.5)];
    [topLine lineToPoint:NSMakePoint(NSWidth(self.bounds), 0.5)];
    [topLine setLineWidth:0.5];
    [topLine stroke];

    if (!_engine) return;

    SCWorkbook *wb = _engine->workbook;
    int activeIdx = wb->active_sheet;

    NSDictionary *inactiveAttrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:11],
        NSForegroundColorAttributeName: [NSColor secondaryLabelColor],
        NSParagraphStyleAttributeName: ({
            NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
            [ps setAlignment:NSTextAlignmentCenter];
            ps;
        })
    };

    NSDictionary *activeAttrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:11 weight:NSFontWeightSemibold],
        NSForegroundColorAttributeName: [NSColor labelColor],
        NSParagraphStyleAttributeName: inactiveAttrs[NSParagraphStyleAttributeName]
    };

    CGFloat x = kLeftPadding;

    for (int i = 0; i < wb->sheet_count; i++) {
        SCSheet *sheet = wb->sheets[i];
        NSRect tabRect = NSMakeRect(x, 2, kTabWidth, kTabHeight);

        if (i == activeIdx) {
            /* Active tab: slightly raised with background */
            [[NSColor controlBackgroundColor] setFill];
            NSBezierPath *tabPath = [NSBezierPath bezierPathWithRoundedRect:tabRect
                                                                    xRadius:4
                                                                    yRadius:4];
            [tabPath fill];

            [[NSColor separatorColor] setStroke];
            [tabPath setLineWidth:0.5];
            [tabPath stroke];
        }

        NSString *name = [NSString stringWithUTF8String:sheet->name];
        NSRect textRect = NSInsetRect(tabRect, 8, 4);
        [name drawInRect:textRect withAttributes:(i == activeIdx) ? activeAttrs : inactiveAttrs];

        x += kTabWidth + 4;
    }

    /* "+" add sheet button */
    NSRect addRect = NSMakeRect(x, 2, kAddButtonWidth, kTabHeight);
    NSDictionary *addAttrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:16 weight:NSFontWeightLight],
        NSForegroundColorAttributeName: [NSColor secondaryLabelColor],
        NSParagraphStyleAttributeName: inactiveAttrs[NSParagraphStyleAttributeName]
    };
    [@"+" drawInRect:NSInsetRect(addRect, 2, 2) withAttributes:addAttrs];
}

- (void)mouseDown:(NSEvent *)event {
    if (!_engine) return;

    NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
    SCWorkbook *wb = _engine->workbook;

    CGFloat x = kLeftPadding;
    for (int i = 0; i < wb->sheet_count; i++) {
        NSRect tabRect = NSMakeRect(x, 2, kTabWidth, kTabHeight);
        if (NSPointInRect(loc, tabRect)) {
            sc_workbook_set_active_sheet(wb, i);
            [self setNeedsDisplay:YES];
            /* Notify grid to redraw */
            [[NSNotificationCenter defaultCenter]
                postNotificationName:@"GLDocumentDidChange" object:nil];
            return;
        }
        x += kTabWidth + 4;
    }

    /* Check add button */
    NSRect addRect = NSMakeRect(x, 2, kAddButtonWidth, kTabHeight);
    if (NSPointInRect(loc, addRect)) {
        sc_workbook_add_sheet(wb, NULL);
        sc_workbook_set_active_sheet(wb, wb->sheet_count - 1);
        [self setNeedsDisplay:YES];
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"GLDocumentDidChange" object:nil];
    }
}

@end
