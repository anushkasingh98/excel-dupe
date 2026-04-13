#import "GLDocument.h"
#import "GLWindowController.h"

@implementation GLDocument {
    SCEngine *_engine;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _engine = sc_engine_create();
    }
    return self;
}

- (void)dealloc {
    sc_engine_destroy(_engine);
}

- (SCEngine *)engine {
    return _engine;
}

- (SCSheet *)activeSheet {
    return sc_engine_active_sheet(_engine);
}

- (void)setCellValue:(const char *)value atRow:(uint32_t)row col:(uint16_t)col {
    sc_engine_set_cell_value(_engine, row, col, value);
    [self updateChangeCount:NSChangeDone];
}

- (void)makeWindowControllers {
    GLWindowController *wc = [[GLWindowController alloc] init];
    [self addWindowController:wc];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    /* TODO: Phase 3 — serialize to xlsx/csv */
    if (outError) {
        *outError = [NSError errorWithDomain:NSCocoaErrorDomain
                                        code:NSFileWriteUnsupportedSchemeError
                                    userInfo:nil];
    }
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    /* TODO: Phase 3 — deserialize from xlsx/csv */
    if (outError) {
        *outError = [NSError errorWithDomain:NSCocoaErrorDomain
                                        code:NSFileReadUnsupportedSchemeError
                                    userInfo:nil];
    }
    return NO;
}

/* Undo/Redo wired to engine */
- (void)undo:(id)sender {
    if (sc_engine_undo(_engine)) {
        [self updateChangeCount:NSChangeUndone];
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"GLDocumentDidChange" object:self];
    }
}

- (void)redo:(id)sender {
    if (sc_engine_redo(_engine)) {
        [self updateChangeCount:NSChangeRedone];
        [[NSNotificationCenter defaultCenter]
            postNotificationName:@"GLDocumentDidChange" object:self];
    }
}

@end
