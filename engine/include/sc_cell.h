#ifndef SC_CELL_H
#define SC_CELL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Cell key: packed (row, col) for hash map storage */
typedef struct {
    uint32_t row;
    uint16_t col;
} SCCellKey;

/* Pack a cell key into a 64-bit integer for hashing */
static inline uint64_t sc_cellkey_pack(SCCellKey key) {
    return ((uint64_t)key.row << 16) | (uint64_t)key.col;
}

static inline SCCellKey sc_cellkey_make(uint32_t row, uint16_t col) {
    return (SCCellKey){ .row = row, .col = col };
}

static inline bool sc_cellkey_equal(SCCellKey a, SCCellKey b) {
    return a.row == b.row && a.col == b.col;
}

/* Cell value types */
typedef enum {
    SC_CELL_EMPTY = 0,
    SC_CELL_NUMBER,
    SC_CELL_STRING,
    SC_CELL_BOOLEAN,
    SC_CELL_ERROR,
    SC_CELL_FORMULA
} SCCellType;

/* Error types */
typedef enum {
    SC_ERROR_NONE = 0,
    SC_ERROR_DIV0,      /* #DIV/0! */
    SC_ERROR_VALUE,     /* #VALUE! */
    SC_ERROR_REF,       /* #REF! */
    SC_ERROR_NAME,      /* #NAME? */
    SC_ERROR_NUM,       /* #NUM! */
    SC_ERROR_NA,        /* #N/A */
    SC_ERROR_NULL       /* #NULL! */
} SCErrorType;

/* Cell formatting */
typedef struct {
    bool bold;
    bool italic;
    bool underline;
    uint8_t font_size;        /* 0 = default (12) */
    uint32_t font_color;      /* 0xRRGGBB, 0 = default (black) */
    uint32_t bg_color;        /* 0xRRGGBB, 0 = default (none/white) */
    bool has_bg_color;
    uint8_t h_align;          /* 0=default, 1=left, 2=center, 3=right */
    uint8_t v_align;          /* 0=default, 1=top, 2=middle, 3=bottom */
    char *number_format;      /* NULL = General */
} SCCellStyle;

/* A single cell */
typedef struct {
    SCCellType type;
    union {
        double number;
        char *string;         /* heap-allocated, owned */
        bool boolean;
        SCErrorType error;
    } value;
    char *formula_text;       /* NULL if not a formula cell, heap-allocated */
    double computed_value;    /* cached formula result (when type == SC_CELL_FORMULA) */
    SCCellStyle *style;       /* NULL if default style */
} SCCell;

/* Create/destroy */
SCCell *sc_cell_create_empty(void);
SCCell *sc_cell_create_number(double value);
SCCell *sc_cell_create_string(const char *value);
SCCell *sc_cell_create_boolean(bool value);
SCCell *sc_cell_create_error(SCErrorType error);
SCCell *sc_cell_create_formula(const char *formula_text, double computed);

SCCell *sc_cell_clone(const SCCell *cell);
void sc_cell_destroy(SCCell *cell);

/* Style management */
SCCellStyle *sc_cell_style_create(void);
SCCellStyle *sc_cell_style_clone(const SCCellStyle *style);
void sc_cell_style_destroy(SCCellStyle *style);
void sc_cell_set_style(SCCell *cell, SCCellStyle *style);

/* Accessors */
const char *sc_cell_get_display_string(const SCCell *cell, char *buf, size_t buf_size);
const char *sc_error_to_string(SCErrorType error);

/* Limits (matching Excel) */
#define SC_MAX_ROWS    1048576
#define SC_MAX_COLS    16384
#define SC_DEFAULT_COL_WIDTH   100.0
#define SC_DEFAULT_ROW_HEIGHT  21.0

#endif /* SC_CELL_H */
