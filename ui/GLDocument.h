#import <Cocoa/Cocoa.h>
#include "sc_engine.h"

@interface GLDocument : NSDocument

@property (nonatomic, readonly) SCEngine *engine;

- (SCSheet *)activeSheet;
- (void)setCellValue:(const char *)value atRow:(uint32_t)row col:(uint16_t)col;

@end
