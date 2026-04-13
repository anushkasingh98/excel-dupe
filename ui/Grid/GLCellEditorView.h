#import <Cocoa/Cocoa.h>

@class GLGridView;

@interface GLCellEditorView : NSTextField <NSTextFieldDelegate>

@property (nonatomic, weak) GLGridView *gridView;

@end
