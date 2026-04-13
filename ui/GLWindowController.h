#import <Cocoa/Cocoa.h>

@class GLGridView;
@class GLFormulaBarView;
@class GLSheetTabBar;

@interface GLWindowController : NSWindowController

@property (nonatomic, strong) GLGridView *gridView;
@property (nonatomic, strong) GLFormulaBarView *formulaBar;
@property (nonatomic, strong) GLSheetTabBar *sheetTabBar;

@end
