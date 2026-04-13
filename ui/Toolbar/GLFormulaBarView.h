#import <Cocoa/Cocoa.h>

@class GLGridView;

@interface GLFormulaBarView : NSView

@property (nonatomic, strong) NSTextField *nameBox;      /* Shows "A1" */
@property (nonatomic, strong) NSTextField *formulaField;  /* Shows formula or value */

- (void)updateFromGridView:(GLGridView *)gridView;

@end
