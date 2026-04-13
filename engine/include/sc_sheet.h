#ifndef SC_SHEET_H
#define SC_SHEET_H

#include "sc_cell.h"
#include "sc_hashmap.h"
#include <stdint.h>

typedef struct SCSheet {
    char *name;
    SCHashMap *cells;            /* SCCellKey -> SCCell* */
    SCHashMap *col_widths;       /* uint16_t col -> double */
    SCHashMap *row_heights;      /* uint32_t row -> double */
    double default_col_width;
    double default_row_height;
    uint32_t used_row_min;
    uint32_t used_row_max;
    uint16_t used_col_min;
    uint16_t used_col_max;
    bool used_range_valid;
} SCSheet;

/* Create/destroy */
SCSheet *sc_sheet_create(const char *name);
void sc_sheet_destroy(SCSheet *sheet);

/* Cell operations */
SCCell *sc_sheet_get_cell(const SCSheet *sheet, uint32_t row, uint16_t col);
void sc_sheet_set_cell(SCSheet *sheet, uint32_t row, uint16_t col, SCCell *cell);
void sc_sheet_remove_cell(SCSheet *sheet, uint32_t row, uint16_t col);

/* Convenience setters (create cell internally) */
void sc_sheet_set_number(SCSheet *sheet, uint32_t row, uint16_t col, double value);
void sc_sheet_set_string(SCSheet *sheet, uint32_t row, uint16_t col, const char *value);
void sc_sheet_set_boolean(SCSheet *sheet, uint32_t row, uint16_t col, bool value);
void sc_sheet_set_formula(SCSheet *sheet, uint32_t row, uint16_t col,
                          const char *formula, double computed);

/* Column/row dimensions */
double sc_sheet_get_col_width(const SCSheet *sheet, uint16_t col);
void sc_sheet_set_col_width(SCSheet *sheet, uint16_t col, double width);
double sc_sheet_get_row_height(const SCSheet *sheet, uint32_t row);
void sc_sheet_set_row_height(SCSheet *sheet, uint32_t row, double height);

/* Used range */
void sc_sheet_recalc_used_range(SCSheet *sheet);
bool sc_sheet_get_used_range(const SCSheet *sheet,
                             uint32_t *row_min, uint16_t *col_min,
                             uint32_t *row_max, uint16_t *col_max);

/* Iteration */
size_t sc_sheet_cell_count(const SCSheet *sheet);

/* Utility: column index <-> letter conversion */
void sc_col_to_letter(uint16_t col, char *buf, size_t buf_size);
int sc_letter_to_col(const char *letters);
void sc_cellkey_to_ref(SCCellKey key, char *buf, size_t buf_size);
bool sc_ref_to_cellkey(const char *ref, SCCellKey *out_key);

#endif /* SC_SHEET_H */
