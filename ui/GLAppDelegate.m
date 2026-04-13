#import "GLAppDelegate.h"
#import "GLDocument.h"

@implementation GLAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [self setupMainMenu];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
    return YES;
}

- (void)setupMainMenu {
    NSMenu *mainMenu = [[NSMenu alloc] init];

    /* App menu */
    NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
    NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"GridLiberty"];
    [appMenu addItemWithTitle:@"About GridLiberty"
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:@"Quit GridLiberty"
                       action:@selector(terminate:)
                keyEquivalent:@"q"];
    [appMenuItem setSubmenu:appMenu];
    [mainMenu addItem:appMenuItem];

    /* File menu */
    NSMenuItem *fileMenuItem = [[NSMenuItem alloc] init];
    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    [fileMenu addItemWithTitle:@"New"
                        action:@selector(newDocument:)
                 keyEquivalent:@"n"];
    [fileMenu addItemWithTitle:@"Open..."
                        action:@selector(openDocument:)
                 keyEquivalent:@"o"];
    [fileMenu addItem:[NSMenuItem separatorItem]];
    [fileMenu addItemWithTitle:@"Save"
                        action:@selector(saveDocument:)
                 keyEquivalent:@"s"];
    NSMenuItem *saveAsItem = [fileMenu addItemWithTitle:@"Save As..."
                                                action:@selector(saveDocumentAs:)
                                         keyEquivalent:@"S"];
    [saveAsItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagShift];
    [fileMenu addItem:[NSMenuItem separatorItem]];
    [fileMenu addItemWithTitle:@"Close"
                        action:@selector(performClose:)
                 keyEquivalent:@"w"];
    [fileMenuItem setSubmenu:fileMenu];
    [mainMenu addItem:fileMenuItem];

    /* Edit menu */
    NSMenuItem *editMenuItem = [[NSMenuItem alloc] init];
    NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
    [editMenu addItemWithTitle:@"Undo"
                        action:@selector(undo:)
                 keyEquivalent:@"z"];
    NSMenuItem *redoItem = [editMenu addItemWithTitle:@"Redo"
                                               action:@selector(redo:)
                                        keyEquivalent:@"Z"];
    [redoItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagShift];
    [editMenu addItem:[NSMenuItem separatorItem]];
    [editMenu addItemWithTitle:@"Cut"
                        action:@selector(cut:)
                 keyEquivalent:@"x"];
    [editMenu addItemWithTitle:@"Copy"
                        action:@selector(copy:)
                 keyEquivalent:@"c"];
    [editMenu addItemWithTitle:@"Paste"
                        action:@selector(paste:)
                 keyEquivalent:@"v"];
    [editMenu addItemWithTitle:@"Delete"
                        action:@selector(delete:)
                 keyEquivalent:@""];
    [editMenu addItem:[NSMenuItem separatorItem]];
    [editMenu addItemWithTitle:@"Select All"
                        action:@selector(selectAll:)
                 keyEquivalent:@"a"];
    [editMenuItem setSubmenu:editMenu];
    [mainMenu addItem:editMenuItem];

    /* View menu */
    NSMenuItem *viewMenuItem = [[NSMenuItem alloc] init];
    NSMenu *viewMenu = [[NSMenu alloc] initWithTitle:@"View"];
    [viewMenu addItemWithTitle:@"Show Formula Bar"
                        action:@selector(toggleFormulaBar:)
                 keyEquivalent:@""];
    [viewMenu addItemWithTitle:@"Show Sheet Tabs"
                        action:@selector(toggleSheetTabs:)
                 keyEquivalent:@""];
    [viewMenuItem setSubmenu:viewMenu];
    [mainMenu addItem:viewMenuItem];

    /* Insert menu */
    NSMenuItem *insertMenuItem = [[NSMenuItem alloc] init];
    NSMenu *insertMenu = [[NSMenu alloc] initWithTitle:@"Insert"];
    [insertMenu addItemWithTitle:@"Row Above"
                          action:@selector(insertRowAbove:)
                   keyEquivalent:@""];
    [insertMenu addItemWithTitle:@"Row Below"
                          action:@selector(insertRowBelow:)
                   keyEquivalent:@""];
    [insertMenu addItemWithTitle:@"Column Left"
                          action:@selector(insertColumnLeft:)
                   keyEquivalent:@""];
    [insertMenu addItemWithTitle:@"Column Right"
                          action:@selector(insertColumnRight:)
                   keyEquivalent:@""];
    [insertMenu addItem:[NSMenuItem separatorItem]];
    [insertMenu addItemWithTitle:@"New Sheet"
                          action:@selector(addSheet:)
                   keyEquivalent:@""];
    [insertMenuItem setSubmenu:insertMenu];
    [mainMenu addItem:insertMenuItem];

    /* Format menu */
    NSMenuItem *formatMenuItem = [[NSMenuItem alloc] init];
    NSMenu *formatMenu = [[NSMenu alloc] initWithTitle:@"Format"];
    [formatMenu addItemWithTitle:@"Bold"
                          action:@selector(toggleBold:)
                   keyEquivalent:@"b"];
    [formatMenu addItemWithTitle:@"Italic"
                          action:@selector(toggleItalic:)
                   keyEquivalent:@"i"];
    [formatMenu addItemWithTitle:@"Underline"
                          action:@selector(toggleUnderline:)
                   keyEquivalent:@"u"];
    [formatMenuItem setSubmenu:formatMenu];
    [mainMenu addItem:formatMenuItem];

    /* Window menu */
    NSMenuItem *windowMenuItem = [[NSMenuItem alloc] init];
    NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [windowMenu addItemWithTitle:@"Minimize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [mainMenu addItem:windowMenuItem];
    [NSApp setWindowsMenu:windowMenu];

    /* Help menu */
    NSMenuItem *helpMenuItem = [[NSMenuItem alloc] init];
    NSMenu *helpMenu = [[NSMenu alloc] initWithTitle:@"Help"];
    [helpMenu addItemWithTitle:@"GridLiberty Help"
                        action:@selector(showHelp:)
                 keyEquivalent:@"?"];
    [helpMenuItem setSubmenu:helpMenu];
    [mainMenu addItem:helpMenuItem];
    [NSApp setHelpMenu:helpMenu];

    [NSApp setMainMenu:mainMenu];
}

@end
