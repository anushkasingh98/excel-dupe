# GridLiberty

A free, native macOS spreadsheet application built from scratch in C and Objective-C. Designed as a fast, clean alternative to Microsoft Excel for Mac users.

## Current Status — Phase 1 Complete

The core spreadsheet engine is fully functional and tested. The native macOS UI is implemented and ready to build on macOS.

### What's Built

**C Engine (`libsc_engine.a`)**

| Module | Description |
|--------|-------------|
| `sc_hashmap` | Robin Hood hash map with FNV-1a hashing |
| `sc_cell` | Cell types: number, string, boolean, error, formula — with styling (bold, italic, colors, alignment) |
| `sc_sheet` | Sparse cell storage keyed on (row, col) — only occupied cells consume memory |
| `sc_workbook` | Multi-sheet management: add, remove, rename, duplicate, reorder |
| `sc_selection` | Single cell, range, and multi-range selection with Shift/Cmd support |
| `sc_undo` | Command-pattern undo/redo with 100-action stack |
| `sc_engine` | Top-level facade combining workbook + selection + undo |

**macOS UI (Objective-C / Cocoa)**

| Component | Description |
|-----------|-------------|
| `GLGridView` | Custom NSView rendering cells via Core Graphics, keyboard navigation, mouse selection, in-place editing |
| `GLDocument` | NSDocument subclass — native Cmd+N/O/S support |
| `GLAppDelegate` | Full native menu bar: File, Edit, View, Insert, Format, Window, Help |
| `GLFormulaBarView` | Name box (cell reference) + formula text field |
| `GLSheetTabBar` | Clickable sheet tabs with "+" button to add sheets |
| `GLColumnHeaderView` | Column letters (A, B, C...) synchronized with grid scroll |
| `GLRowHeaderView` | Row numbers (1, 2, 3...) synchronized with grid scroll |

### What's Coming

- **Phase 2**: Formula engine (hand-built recursive descent parser, `=SUM(A1:A10)`, dependency graph, ~30 built-in functions)
- **Phase 3**: Cell formatting, clipboard (Cmd+C/V/X), .xlsx/.csv file I/O
- **Phase 4**: Multi-sheet tabs, row/column insert/delete, cross-sheet references
- **Phase 5**: Find & replace, sorting, auto-filter
- **Phase 6**: Embedded Python scripting for macros and data analysis
- **Phase 7**: Charts, printing, dark mode polish, DMG packaging

## Prerequisites

### For the engine (builds on macOS and Linux)

- CMake 3.24+
- A C17-compatible compiler (GCC 11+ or Clang 14+)

### For the full macOS app (macOS only)

- macOS 12.0+
- Xcode Command Line Tools (provides Clang, Cocoa frameworks)
- Optional: Python 3.10+ development headers (for Phase 6)

Install Xcode CLT if you haven't already:

```bash
xcode-select --install
```

## Building

### Build the engine and run tests (macOS or Linux)

```bash
# Clone the repo
git clone https://github.com/anushkasingh98/excel-dupe.git
cd excel-dupe

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run all tests
ctest --output-on-failure
```

You should see:

```
1/5 Test #1: hashmap ..........................   Passed
2/5 Test #2: cell .............................   Passed
3/5 Test #3: sheet ............................   Passed
4/5 Test #4: undo .............................   Passed
5/5 Test #5: selection ........................   Passed

100% tests passed, 0 tests failed out of 5
```

### Build the full macOS app (macOS only)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# The app bundle is at:
# build/ui/GridLiberty.app

# Launch it:
open ui/GridLiberty.app
```

Or generate an Xcode project for debugging:

```bash
mkdir xcode-build && cd xcode-build
cmake .. -G Xcode
open GridLiberty.xcodeproj
```

## Testing the Engine

You can run individual test suites directly:

```bash
cd build

# Run a specific test
./engine/tests/test_hashmap
./engine/tests/test_cell
./engine/tests/test_sheet
./engine/tests/test_undo
./engine/tests/test_selection

# Or run all via CTest
ctest -V    # verbose output
```

### What the tests cover

| Test Suite | Tests |
|-----------|-------|
| `test_hashmap` | Create/destroy, set/get, overwrite, remove, 1000-entry stress test, iterator, contains, clear |
| `test_cell` | All cell types (number, string, boolean, error, formula, empty), clone with deep copy, display strings, styles, cell key packing |
| `test_sheet` | Cell CRUD, column/row dimensions, used range tracking, column letter conversion (A-ZZ-AAA), cell reference parsing (A1, $A$1, AA100), workbook operations (add/remove/rename/move/duplicate sheets), 10,000-cell stress test |
| `test_undo` | Basic undo/redo, multi-step undo, redo cleared on new action, string/boolean/formula undo, selection clear + undo, stack overflow at 100 actions |
| `test_selection` | Single cell, range, multi-range (Cmd+click), extend (Shift), move with bounds checking, range normalization, cell count, bounding range |

## Project Structure

```
excel-dupe/
├── engine/              # Pure C core (no UI dependencies)
│   ├── include/         # Public headers (sc_cell.h, sc_sheet.h, etc.)
│   ├── src/             # Implementation files
│   └── tests/           # Unit tests
├── ui/                  # Objective-C / Cocoa UI (macOS only)
│   ├── Grid/            # GLGridView, headers, cell editor
│   ├── Toolbar/         # Formula bar
│   └── Sidebar/         # Sheet tab bar
├── python/              # Python bridge (Phase 6 placeholder)
├── CMakeLists.txt       # Top-level build config
└── Info.plist           # macOS app bundle metadata
```

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Core engine | C17 |
| UI | Cocoa (AppKit) via Objective-C |
| Scripting | Embedded CPython (Phase 6) |
| .xlsx writing | libxlsxwriter (Phase 3) |
| .xlsx reading | minizip + expat (Phase 3) |
| Grid rendering | Core Graphics (custom NSView) |
| Build system | CMake |
| Tests | assert-based (cmocka planned) |

## License

All rights reserved.
