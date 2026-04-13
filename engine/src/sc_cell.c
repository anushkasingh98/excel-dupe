#include "sc_cell.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SCCell *sc_cell_create_empty(void) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (cell) cell->type = SC_CELL_EMPTY;
    return cell;
}

SCCell *sc_cell_create_number(double value) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (!cell) return NULL;
    cell->type = SC_CELL_NUMBER;
    cell->value.number = value;
    return cell;
}

SCCell *sc_cell_create_string(const char *value) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (!cell) return NULL;
    cell->type = SC_CELL_STRING;
    cell->value.string = value ? strdup(value) : NULL;
    return cell;
}

SCCell *sc_cell_create_boolean(bool value) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (!cell) return NULL;
    cell->type = SC_CELL_BOOLEAN;
    cell->value.boolean = value;
    return cell;
}

SCCell *sc_cell_create_error(SCErrorType error) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (!cell) return NULL;
    cell->type = SC_CELL_ERROR;
    cell->value.error = error;
    return cell;
}

SCCell *sc_cell_create_formula(const char *formula_text, double computed) {
    SCCell *cell = calloc(1, sizeof(SCCell));
    if (!cell) return NULL;
    cell->type = SC_CELL_FORMULA;
    cell->formula_text = formula_text ? strdup(formula_text) : NULL;
    cell->computed_value = computed;
    return cell;
}

SCCell *sc_cell_clone(const SCCell *cell) {
    if (!cell) return NULL;

    SCCell *clone = calloc(1, sizeof(SCCell));
    if (!clone) return NULL;

    clone->type = cell->type;
    clone->computed_value = cell->computed_value;

    switch (cell->type) {
        case SC_CELL_STRING:
            clone->value.string = cell->value.string ? strdup(cell->value.string) : NULL;
            break;
        case SC_CELL_NUMBER:
            clone->value.number = cell->value.number;
            break;
        case SC_CELL_BOOLEAN:
            clone->value.boolean = cell->value.boolean;
            break;
        case SC_CELL_ERROR:
            clone->value.error = cell->value.error;
            break;
        case SC_CELL_FORMULA:
            clone->value.number = cell->value.number;
            break;
        default:
            break;
    }

    clone->formula_text = cell->formula_text ? strdup(cell->formula_text) : NULL;

    if (cell->style) {
        clone->style = sc_cell_style_clone(cell->style);
    }

    return clone;
}

void sc_cell_destroy(SCCell *cell) {
    if (!cell) return;
    if (cell->type == SC_CELL_STRING) {
        free(cell->value.string);
    }
    free(cell->formula_text);
    sc_cell_style_destroy(cell->style);
    free(cell);
}

/* Style */
SCCellStyle *sc_cell_style_create(void) {
    SCCellStyle *style = calloc(1, sizeof(SCCellStyle));
    return style;
}

SCCellStyle *sc_cell_style_clone(const SCCellStyle *style) {
    if (!style) return NULL;
    SCCellStyle *clone = malloc(sizeof(SCCellStyle));
    if (!clone) return NULL;
    memcpy(clone, style, sizeof(SCCellStyle));
    clone->number_format = style->number_format ? strdup(style->number_format) : NULL;
    return clone;
}

void sc_cell_style_destroy(SCCellStyle *style) {
    if (!style) return;
    free(style->number_format);
    free(style);
}

void sc_cell_set_style(SCCell *cell, SCCellStyle *style) {
    if (!cell) return;
    sc_cell_style_destroy(cell->style);
    cell->style = style;
}

const char *sc_error_to_string(SCErrorType error) {
    switch (error) {
        case SC_ERROR_NONE:  return "";
        case SC_ERROR_DIV0:  return "#DIV/0!";
        case SC_ERROR_VALUE: return "#VALUE!";
        case SC_ERROR_REF:   return "#REF!";
        case SC_ERROR_NAME:  return "#NAME?";
        case SC_ERROR_NUM:   return "#NUM!";
        case SC_ERROR_NA:    return "#N/A";
        case SC_ERROR_NULL:  return "#NULL!";
    }
    return "#ERROR";
}

const char *sc_cell_get_display_string(const SCCell *cell, char *buf, size_t buf_size) {
    if (!cell || buf_size == 0) return "";

    switch (cell->type) {
        case SC_CELL_EMPTY:
            buf[0] = '\0';
            break;
        case SC_CELL_NUMBER:
            snprintf(buf, buf_size, "%g", cell->value.number);
            break;
        case SC_CELL_STRING:
            snprintf(buf, buf_size, "%s", cell->value.string ? cell->value.string : "");
            break;
        case SC_CELL_BOOLEAN:
            snprintf(buf, buf_size, "%s", cell->value.boolean ? "TRUE" : "FALSE");
            break;
        case SC_CELL_ERROR:
            snprintf(buf, buf_size, "%s", sc_error_to_string(cell->value.error));
            break;
        case SC_CELL_FORMULA:
            snprintf(buf, buf_size, "%g", cell->computed_value);
            break;
    }
    return buf;
}
