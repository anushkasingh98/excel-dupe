#include "sc_sheet.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Helper: free a cell stored as a pointer in the hashmap value slot */
static void free_cell_ptr(void *ptr) {
    sc_cell_destroy((SCCell *)ptr);
}

SCSheet *sc_sheet_create(const char *name) {
    SCSheet *sheet = calloc(1, sizeof(SCSheet));
    if (!sheet) return NULL;

    sheet->name = name ? strdup(name) : strdup("Sheet1");
    sheet->cells = sc_hashmap_create(sizeof(SCCellKey), sizeof(SCCell *), NULL, NULL);
    sheet->col_widths = sc_hashmap_create(sizeof(uint16_t), sizeof(double), NULL, NULL);
    sheet->row_heights = sc_hashmap_create(sizeof(uint32_t), sizeof(double), NULL, NULL);
    sheet->default_col_width = SC_DEFAULT_COL_WIDTH;
    sheet->default_row_height = SC_DEFAULT_ROW_HEIGHT;
    sheet->used_range_valid = false;

    if (!sheet->cells || !sheet->col_widths || !sheet->row_heights) {
        sc_sheet_destroy(sheet);
        return NULL;
    }
    return sheet;
}

void sc_sheet_destroy(SCSheet *sheet) {
    if (!sheet) return;
    free(sheet->name);
    sc_hashmap_destroy(sheet->cells, free_cell_ptr);
    sc_hashmap_destroy(sheet->col_widths, NULL);
    sc_hashmap_destroy(sheet->row_heights, NULL);
    free(sheet);
}

SCCell *sc_sheet_get_cell(const SCSheet *sheet, uint32_t row, uint16_t col) {
    if (!sheet) return NULL;
    SCCellKey key = sc_cellkey_make(row, col);
    void *val = sc_hashmap_get(sheet->cells, &key);
    if (!val) return NULL;
    SCCell *cell;
    memcpy(&cell, val, sizeof(SCCell *));
    return cell;
}

void sc_sheet_set_cell(SCSheet *sheet, uint32_t row, uint16_t col, SCCell *cell) {
    if (!sheet || !cell) return;

    SCCellKey key = sc_cellkey_make(row, col);

    /* Remove old cell if exists */
    void *old_val = sc_hashmap_get(sheet->cells, &key);
    if (old_val) {
        SCCell *old_cell;
        memcpy(&old_cell, old_val, sizeof(SCCell *));
        sc_cell_destroy(old_cell);
    }

    sc_hashmap_set(sheet->cells, &key, &cell);
    sheet->used_range_valid = false;
}

void sc_sheet_remove_cell(SCSheet *sheet, uint32_t row, uint16_t col) {
    if (!sheet) return;
    SCCellKey key = sc_cellkey_make(row, col);
    sc_hashmap_remove(sheet->cells, &key, free_cell_ptr);
    sheet->used_range_valid = false;
}

void sc_sheet_set_number(SCSheet *sheet, uint32_t row, uint16_t col, double value) {
    sc_sheet_set_cell(sheet, row, col, sc_cell_create_number(value));
}

void sc_sheet_set_string(SCSheet *sheet, uint32_t row, uint16_t col, const char *value) {
    sc_sheet_set_cell(sheet, row, col, sc_cell_create_string(value));
}

void sc_sheet_set_boolean(SCSheet *sheet, uint32_t row, uint16_t col, bool value) {
    sc_sheet_set_cell(sheet, row, col, sc_cell_create_boolean(value));
}

void sc_sheet_set_formula(SCSheet *sheet, uint32_t row, uint16_t col,
                          const char *formula, double computed) {
    sc_sheet_set_cell(sheet, row, col, sc_cell_create_formula(formula, computed));
}

/* Column/row dimensions */
double sc_sheet_get_col_width(const SCSheet *sheet, uint16_t col) {
    if (!sheet) return SC_DEFAULT_COL_WIDTH;
    void *val = sc_hashmap_get(sheet->col_widths, &col);
    if (!val) return sheet->default_col_width;
    double width;
    memcpy(&width, val, sizeof(double));
    return width;
}

void sc_sheet_set_col_width(SCSheet *sheet, uint16_t col, double width) {
    if (!sheet) return;
    sc_hashmap_set(sheet->col_widths, &col, &width);
}

double sc_sheet_get_row_height(const SCSheet *sheet, uint32_t row) {
    if (!sheet) return SC_DEFAULT_ROW_HEIGHT;
    void *val = sc_hashmap_get(sheet->row_heights, &row);
    if (!val) return sheet->default_row_height;
    double height;
    memcpy(&height, val, sizeof(double));
    return height;
}

void sc_sheet_set_row_height(SCSheet *sheet, uint32_t row, double height) {
    if (!sheet) return;
    sc_hashmap_set(sheet->row_heights, &row, &height);
}

/* Used range */
void sc_sheet_recalc_used_range(SCSheet *sheet) {
    if (!sheet) return;

    if (sc_hashmap_count(sheet->cells) == 0) {
        sheet->used_row_min = 0;
        sheet->used_row_max = 0;
        sheet->used_col_min = 0;
        sheet->used_col_max = 0;
        sheet->used_range_valid = true;
        return;
    }

    uint32_t rmin = UINT32_MAX, rmax = 0;
    uint16_t cmin = UINT16_MAX, cmax = 0;

    SCHashMapIter iter;
    sc_hashmap_iter_init(&iter, sheet->cells);
    void *key_ptr, *val_ptr;
    while (sc_hashmap_iter_next(&iter, &key_ptr, &val_ptr)) {
        SCCellKey key;
        memcpy(&key, key_ptr, sizeof(SCCellKey));
        if (key.row < rmin) rmin = key.row;
        if (key.row > rmax) rmax = key.row;
        if (key.col < cmin) cmin = key.col;
        if (key.col > cmax) cmax = key.col;
    }

    sheet->used_row_min = rmin;
    sheet->used_row_max = rmax;
    sheet->used_col_min = cmin;
    sheet->used_col_max = cmax;
    sheet->used_range_valid = true;
}

bool sc_sheet_get_used_range(const SCSheet *sheet,
                             uint32_t *row_min, uint16_t *col_min,
                             uint32_t *row_max, uint16_t *col_max) {
    if (!sheet || sc_hashmap_count(sheet->cells) == 0) return false;

    /* Recalc if needed — cast away const for cache update */
    if (!sheet->used_range_valid) {
        sc_sheet_recalc_used_range((SCSheet *)sheet);
    }

    if (row_min) *row_min = sheet->used_row_min;
    if (col_min) *col_min = sheet->used_col_min;
    if (row_max) *row_max = sheet->used_row_max;
    if (col_max) *col_max = sheet->used_col_max;
    return true;
}

size_t sc_sheet_cell_count(const SCSheet *sheet) {
    return sheet ? sc_hashmap_count(sheet->cells) : 0;
}

/* Column index to letter(s): 0 -> "A", 25 -> "Z", 26 -> "AA", etc. */
void sc_col_to_letter(uint16_t col, char *buf, size_t buf_size) {
    if (buf_size == 0) return;

    char tmp[8];
    int i = 0;
    uint16_t c = col;

    do {
        tmp[i++] = 'A' + (c % 26);
        c = c / 26 - 1;
    } while (c < UINT16_MAX && i < 7);
    /* c wraps around when original col < 26, causing the loop to end */

    /* Reverse into buf */
    int j = 0;
    for (int k = i - 1; k >= 0 && j < (int)buf_size - 1; k--) {
        buf[j++] = tmp[k];
    }
    buf[j] = '\0';
}

int sc_letter_to_col(const char *letters) {
    if (!letters) return -1;
    int col = 0;
    for (int i = 0; letters[i] && isalpha((unsigned char)letters[i]); i++) {
        col = col * 26 + (toupper((unsigned char)letters[i]) - 'A' + 1);
    }
    return col - 1; /* 0-indexed */
}

void sc_cellkey_to_ref(SCCellKey key, char *buf, size_t buf_size) {
    if (buf_size < 8) { if (buf_size > 0) buf[0] = '\0'; return; }
    char col_buf[8];
    sc_col_to_letter(key.col, col_buf, sizeof(col_buf));
    snprintf(buf, buf_size, "%s%u", col_buf, key.row + 1);
}

bool sc_ref_to_cellkey(const char *ref, SCCellKey *out_key) {
    if (!ref || !out_key) return false;

    /* Skip optional $ for absolute reference */
    const char *p = ref;
    if (*p == '$') p++;

    /* Parse column letters */
    if (!isalpha((unsigned char)*p)) return false;
    int col = 0;
    while (isalpha((unsigned char)*p)) {
        col = col * 26 + (toupper((unsigned char)*p) - 'A' + 1);
        p++;
    }
    col--; /* 0-indexed */

    /* Skip optional $ */
    if (*p == '$') p++;

    /* Parse row number */
    if (!isdigit((unsigned char)*p)) return false;
    long row = strtol(p, NULL, 10);
    if (row < 1 || row > SC_MAX_ROWS) return false;

    out_key->row = (uint32_t)(row - 1);
    out_key->col = (uint16_t)col;
    return true;
}
