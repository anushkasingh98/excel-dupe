#include "sc_workbook.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SCWorkbook *sc_workbook_create(void) {
    SCWorkbook *wb = calloc(1, sizeof(SCWorkbook));
    if (!wb) return NULL;

    /* Start with one default sheet */
    wb->sheets[0] = sc_sheet_create("Sheet1");
    if (!wb->sheets[0]) {
        free(wb);
        return NULL;
    }
    wb->sheet_count = 1;
    wb->active_sheet = 0;
    return wb;
}

void sc_workbook_destroy(SCWorkbook *wb) {
    if (!wb) return;
    for (int i = 0; i < wb->sheet_count; i++) {
        sc_sheet_destroy(wb->sheets[i]);
    }
    free(wb);
}

SCSheet *sc_workbook_add_sheet(SCWorkbook *wb, const char *name) {
    if (!wb || wb->sheet_count >= SC_MAX_SHEETS) return NULL;

    /* Generate name if NULL */
    char auto_name[32];
    if (!name) {
        snprintf(auto_name, sizeof(auto_name), "Sheet%d", wb->sheet_count + 1);
        name = auto_name;
    }

    SCSheet *sheet = sc_sheet_create(name);
    if (!sheet) return NULL;

    wb->sheets[wb->sheet_count] = sheet;
    wb->sheet_count++;
    return sheet;
}

bool sc_workbook_remove_sheet(SCWorkbook *wb, int index) {
    if (!wb || index < 0 || index >= wb->sheet_count) return false;
    if (wb->sheet_count <= 1) return false; /* must keep at least one sheet */

    sc_sheet_destroy(wb->sheets[index]);

    /* Shift remaining sheets left */
    for (int i = index; i < wb->sheet_count - 1; i++) {
        wb->sheets[i] = wb->sheets[i + 1];
    }
    wb->sheets[wb->sheet_count - 1] = NULL;
    wb->sheet_count--;

    /* Adjust active sheet index */
    if (wb->active_sheet >= wb->sheet_count) {
        wb->active_sheet = wb->sheet_count - 1;
    }
    return true;
}

bool sc_workbook_rename_sheet(SCWorkbook *wb, int index, const char *new_name) {
    if (!wb || !new_name || index < 0 || index >= wb->sheet_count) return false;
    free(wb->sheets[index]->name);
    wb->sheets[index]->name = strdup(new_name);
    return true;
}

bool sc_workbook_move_sheet(SCWorkbook *wb, int from_index, int to_index) {
    if (!wb || from_index < 0 || from_index >= wb->sheet_count ||
        to_index < 0 || to_index >= wb->sheet_count) return false;
    if (from_index == to_index) return true;

    SCSheet *moving = wb->sheets[from_index];

    if (from_index < to_index) {
        for (int i = from_index; i < to_index; i++) {
            wb->sheets[i] = wb->sheets[i + 1];
        }
    } else {
        for (int i = from_index; i > to_index; i--) {
            wb->sheets[i] = wb->sheets[i - 1];
        }
    }
    wb->sheets[to_index] = moving;
    return true;
}

SCSheet *sc_workbook_duplicate_sheet(SCWorkbook *wb, int index, const char *new_name) {
    if (!wb || index < 0 || index >= wb->sheet_count) return NULL;
    if (wb->sheet_count >= SC_MAX_SHEETS) return NULL;

    SCSheet *src = wb->sheets[index];

    /* Generate name if not provided */
    char auto_name[64];
    if (!new_name) {
        snprintf(auto_name, sizeof(auto_name), "%s (Copy)", src->name);
        new_name = auto_name;
    }

    SCSheet *dup = sc_sheet_create(new_name);
    if (!dup) return NULL;

    /* Copy all cells */
    SCHashMapIter iter;
    sc_hashmap_iter_init(&iter, src->cells);
    void *key_ptr, *val_ptr;
    while (sc_hashmap_iter_next(&iter, &key_ptr, &val_ptr)) {
        SCCellKey key;
        memcpy(&key, key_ptr, sizeof(SCCellKey));
        SCCell *cell;
        memcpy(&cell, val_ptr, sizeof(SCCell *));
        SCCell *clone = sc_cell_clone(cell);
        if (clone) {
            sc_sheet_set_cell(dup, key.row, key.col, clone);
        }
    }

    /* Copy column widths */
    sc_hashmap_iter_init(&iter, src->col_widths);
    while (sc_hashmap_iter_next(&iter, &key_ptr, &val_ptr)) {
        uint16_t col;
        double width;
        memcpy(&col, key_ptr, sizeof(uint16_t));
        memcpy(&width, val_ptr, sizeof(double));
        sc_sheet_set_col_width(dup, col, width);
    }

    /* Copy row heights */
    sc_hashmap_iter_init(&iter, src->row_heights);
    while (sc_hashmap_iter_next(&iter, &key_ptr, &val_ptr)) {
        uint32_t row;
        double height;
        memcpy(&row, key_ptr, sizeof(uint32_t));
        memcpy(&height, val_ptr, sizeof(double));
        sc_sheet_set_row_height(dup, row, height);
    }

    wb->sheets[wb->sheet_count] = dup;
    wb->sheet_count++;
    return dup;
}

SCSheet *sc_workbook_get_sheet(const SCWorkbook *wb, int index) {
    if (!wb || index < 0 || index >= wb->sheet_count) return NULL;
    return wb->sheets[index];
}

SCSheet *sc_workbook_get_sheet_by_name(const SCWorkbook *wb, const char *name) {
    if (!wb || !name) return NULL;
    for (int i = 0; i < wb->sheet_count; i++) {
        if (strcmp(wb->sheets[i]->name, name) == 0) {
            return wb->sheets[i];
        }
    }
    return NULL;
}

int sc_workbook_get_sheet_index(const SCWorkbook *wb, const char *name) {
    if (!wb || !name) return -1;
    for (int i = 0; i < wb->sheet_count; i++) {
        if (strcmp(wb->sheets[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int sc_workbook_get_active_sheet(const SCWorkbook *wb) {
    return wb ? wb->active_sheet : 0;
}

void sc_workbook_set_active_sheet(SCWorkbook *wb, int index) {
    if (!wb || index < 0 || index >= wb->sheet_count) return;
    wb->active_sheet = index;
}
