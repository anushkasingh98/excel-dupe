#include "sc_selection.h"
#include <stdlib.h>
#include <string.h>

SCSelection *sc_selection_create(void) {
    SCSelection *sel = calloc(1, sizeof(SCSelection));
    if (!sel) return NULL;
    /* Default: select cell A1 */
    sel->active_cell = sc_cellkey_make(0, 0);
    sel->ranges[0] = (SCRange){ 0, 0, 0, 0 };
    sel->range_count = 1;
    sel->editing = false;
    return sel;
}

void sc_selection_destroy(SCSelection *sel) {
    free(sel);
}

void sc_selection_set_cell(SCSelection *sel, uint32_t row, uint16_t col) {
    if (!sel) return;
    sel->active_cell = sc_cellkey_make(row, col);
    sel->ranges[0] = (SCRange){ row, col, row, col };
    sel->range_count = 1;
}

void sc_selection_set_range(SCSelection *sel,
                            uint32_t start_row, uint16_t start_col,
                            uint32_t end_row, uint16_t end_col) {
    if (!sel) return;
    sel->active_cell = sc_cellkey_make(start_row, start_col);
    sel->ranges[0] = (SCRange){ start_row, start_col, end_row, end_col };
    sel->range_count = 1;
}

void sc_selection_add_range(SCSelection *sel,
                            uint32_t start_row, uint16_t start_col,
                            uint32_t end_row, uint16_t end_col) {
    if (!sel || sel->range_count >= SC_MAX_SELECTION_RANGES) return;
    sel->active_cell = sc_cellkey_make(start_row, start_col);
    sel->ranges[sel->range_count] = (SCRange){ start_row, start_col, end_row, end_col };
    sel->range_count++;
}

void sc_selection_extend_to(SCSelection *sel, uint32_t row, uint16_t col) {
    if (!sel || sel->range_count == 0) return;
    /* Extend the last range from active_cell to the new position */
    SCRange *last = &sel->ranges[sel->range_count - 1];
    last->start_row = sel->active_cell.row;
    last->start_col = sel->active_cell.col;
    last->end_row = row;
    last->end_col = col;
}

void sc_selection_move(SCSelection *sel, int row_delta, int col_delta) {
    if (!sel) return;
    int new_row = (int)sel->active_cell.row + row_delta;
    int new_col = (int)sel->active_cell.col + col_delta;

    if (new_row < 0) new_row = 0;
    if (new_row >= SC_MAX_ROWS) new_row = SC_MAX_ROWS - 1;
    if (new_col < 0) new_col = 0;
    if (new_col >= SC_MAX_COLS) new_col = SC_MAX_COLS - 1;

    sc_selection_set_cell(sel, (uint32_t)new_row, (uint16_t)new_col);
}

bool sc_selection_contains(const SCSelection *sel, uint32_t row, uint16_t col) {
    if (!sel) return false;
    for (int i = 0; i < sel->range_count; i++) {
        if (sc_range_contains(sc_range_normalize(sel->ranges[i]), row, col)) {
            return true;
        }
    }
    return false;
}

SCRange sc_selection_get_bounding_range(const SCSelection *sel) {
    if (!sel || sel->range_count == 0) {
        return (SCRange){ 0, 0, 0, 0 };
    }

    SCRange bounds = sc_range_normalize(sel->ranges[0]);
    for (int i = 1; i < sel->range_count; i++) {
        SCRange r = sc_range_normalize(sel->ranges[i]);
        if (r.start_row < bounds.start_row) bounds.start_row = r.start_row;
        if (r.start_col < bounds.start_col) bounds.start_col = r.start_col;
        if (r.end_row > bounds.end_row) bounds.end_row = r.end_row;
        if (r.end_col > bounds.end_col) bounds.end_col = r.end_col;
    }
    return bounds;
}

SCRange sc_range_normalize(SCRange range) {
    SCRange n = range;
    if (n.start_row > n.end_row) {
        uint32_t tmp = n.start_row;
        n.start_row = n.end_row;
        n.end_row = tmp;
    }
    if (n.start_col > n.end_col) {
        uint16_t tmp = n.start_col;
        n.start_col = n.end_col;
        n.end_col = tmp;
    }
    return n;
}

bool sc_range_contains(SCRange range, uint32_t row, uint16_t col) {
    SCRange n = sc_range_normalize(range);
    return row >= n.start_row && row <= n.end_row &&
           col >= n.start_col && col <= n.end_col;
}

size_t sc_range_cell_count(SCRange range) {
    SCRange n = sc_range_normalize(range);
    return (size_t)(n.end_row - n.start_row + 1) * (n.end_col - n.start_col + 1);
}
