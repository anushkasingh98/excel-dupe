#import "GLWindowController.h"
#import "GLDocument.h"
#import "Grid/GLGridView.h"
#import "Grid/GLColumnHeaderView.h"
#import "Grid/GLRowHeaderView.h"
#import "Toolbar/GLFormulaBarView.h"
#import "Sidebar/GLSheetTabBar.h"

static const CGFloat kRowHeaderWidth = 50.0;
static const CGFloat kColumnHeaderHeight = 24.0;
static const CGFloat kFormulaBarHeight = 28.0;
static const CGFloat kSheetTabBarHeight = 28.0;

@implementation GLWindowController

- (instancetype)init {
    self = [super initWithWindow:nil];
    if (self) {
        [self setupWindow];
    }
    return self;
}

- (void)setupWindow {
    NSRect frame = NSMakeRect(100, 100, 1280, 800);
    NSUInteger styleMask = NSWindowStyleMaskTitled |
                           NSWindowStyleMaskClosable |
                           NSWindowStyleMaskMiniaturizable |
                           NSWindowStyleMaskResizable;

    NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:styleMask
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    [window setTitle:@"GridLiberty"];
    [window setMinSize:NSMakeSize(640, 480)];
    [window center];

    /* Use semantic colors for dark mode support */
    window.backgroundColor = [NSColor windowBackgroundColor];

    self.window = window;
    [self setupContentView];

    /* Observe document changes */
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(documentDidChange:)
               name:@"GLDocumentDidChange"
             object:nil];
}

- (void)setupContentView {
    NSView *contentView = self.window.contentView;
    contentView.autoresizesSubviews = YES;

    NSRect bounds = contentView.bounds;

    /* Formula bar at top */
    NSRect formulaFrame = NSMakeRect(0, NSMaxY(bounds) - kFormulaBarHeight,
                                     NSWidth(bounds), kFormulaBarHeight);
    _formulaBar = [[GLFormulaBarView alloc] initWithFrame:formulaFrame];
    _formulaBar.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
    [contentView addSubview:_formulaBar];

    /* Sheet tab bar at bottom */
    NSRect tabFrame = NSMakeRect(0, 0, NSWidth(bounds), kSheetTabBarHeight);
    _sheetTabBar = [[GLSheetTabBar alloc] initWithFrame:tabFrame];
    _sheetTabBar.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    [contentView addSubview:_sheetTabBar];

    /* Column header */
    CGFloat gridAreaTop = NSMaxY(bounds) - kFormulaBarHeight;
    CGFloat gridAreaBottom = kSheetTabBarHeight;
    CGFloat gridAreaHeight = gridAreaTop - gridAreaBottom;

    NSRect colHeaderFrame = NSMakeRect(kRowHeaderWidth,
                                       gridAreaTop - kColumnHeaderHeight,
                                       NSWidth(bounds) - kRowHeaderWidth,
                                       kColumnHeaderHeight);
    GLColumnHeaderView *colHeader = [[GLColumnHeaderView alloc] initWithFrame:colHeaderFrame];
    colHeader.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
    [contentView addSubview:colHeader];

    /* Row header */
    NSRect rowHeaderFrame = NSMakeRect(0,
                                       gridAreaBottom,
                                       kRowHeaderWidth,
                                       gridAreaHeight - kColumnHeaderHeight);
    GLRowHeaderView *rowHeader = [[GLRowHeaderView alloc] initWithFrame:rowHeaderFrame];
    rowHeader.autoresizingMask = NSViewHeightSizable | NSViewMaxXMargin;
    [contentView addSubview:rowHeader];

    /* Main grid (scrollable) */
    NSRect gridFrame = NSMakeRect(kRowHeaderWidth,
                                   gridAreaBottom,
                                   NSWidth(bounds) - kRowHeaderWidth,
                                   gridAreaHeight - kColumnHeaderHeight);

    NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:gridFrame];
    scrollView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    scrollView.hasVerticalScroller = YES;
    scrollView.hasHorizontalScroller = YES;
    scrollView.borderType = NSNoBorder;

    /* Grid is much larger than the visible area */
    NSRect gridDocFrame = NSMakeRect(0, 0, 16384 * 100, 1000 * 21);
    _gridView = [[GLGridView alloc] initWithFrame:gridDocFrame];
    scrollView.documentView = _gridView;

    [contentView addSubview:scrollView];

    /* Wire up cross-references */
    _gridView.columnHeaderView = colHeader;
    _gridView.rowHeaderView = rowHeader;
    _gridView.formulaBar = _formulaBar;
    colHeader.gridView = _gridView;
    rowHeader.gridView = _gridView;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    [self syncGridToDocument];
}

- (void)setDocument:(id)document {
    [super setDocument:document];
    [self syncGridToDocument];
}

- (void)syncGridToDocument {
    GLDocument *doc = self.document;
    if (doc && _gridView) {
        _gridView.engine = doc.engine;
        [_gridView setNeedsDisplay:YES];
        [_sheetTabBar setEngine:doc.engine];
        [_sheetTabBar setNeedsDisplay:YES];
    }
}

- (void)documentDidChange:(NSNotification *)note {
    if (note.object == self.document) {
        [_gridView setNeedsDisplay:YES];
        [_formulaBar updateFromGridView:_gridView];
    }
}

@end
