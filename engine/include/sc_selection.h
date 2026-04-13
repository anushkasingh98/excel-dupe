#ifndef SC_SELECTION_H
#define SC_SELECTION_H

#include "sc_cell.h"
#include <stdbool.h>

/* A rectangular range of cells */
typedef struct {
    uint32_t start_row;
    uint16_t start_col;
    uint32_t end_row;
    uint16_t end_col;
} SCRange;

#define SC_MAX_SELECTION_RANGES 64

typedef struct {
    SCCellKey active_cell;       /* The single "active" cell (blue border) */
    SCRange ranges[SC_MAX_SELECTION_RANGES];
    int range_count;
    bool editing;                /* True when cell editor is open */
} SCSelection;

/* Create/destroy */
SCSelection *sc_selection_create(void);
void sc_selection_destroy(SCSelection *sel);

/* Set active cell (clears all ranges, creates a single-cell range) */
void sc_selection_set_cell(SCSelection *sel, uint32_t row, uint16_t col);

/* Set a single range selection */
void sc_selection_set_range(SCSelection *sel,
                            uint32_t start_row, uint16_t start_col,
                            uint32_t end_row, uint16_t end_col);

/* Add an additional range (Cmd+click) */
void sc_selection_add_range(SCSelection *sel,
                            uint32_t start_row, uint16_t start_col,
                            uint32_t end_row, uint16_t end_col);

/* Extend the last range (Shift+click or Shift+Arrow) */
void sc_selection_extend_to(SCSelection *sel, uint32_t row, uint16_t col);

/* Navigation */
void sc_selection_move(SCSelection *sel, int row_delta, int col_delta);
void sc_selection_move_to_edge(SCSelection *sel, int row_dir, int col_dir);

/* Query */
bool sc_selection_contains(const SCSelection *sel, uint32_t row, uint16_t col);
SCRange sc_selection_get_bounding_range(const SCSelection *sel);

/* Normalize a range so start <= end */
SCRange sc_range_normalize(SCRange range);
bool sc_range_contains(SCRange range, uint32_t row, uint16_t col);
size_t sc_range_cell_count(SCRange range);

#endif /* SC_SELECTION_H */
