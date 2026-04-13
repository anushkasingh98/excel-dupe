#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sc_cell.h"

static void test_create_number(void) {
    SCCell *cell = sc_cell_create_number(42.5);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_NUMBER);
    assert(cell->value.number == 42.5);
    assert(cell->formula_text == NULL);
    assert(cell->style == NULL);
    sc_cell_destroy(cell);
    printf("  PASS: create number\n");
}

static void test_create_string(void) {
    SCCell *cell = sc_cell_create_string("Hello");
    assert(cell != NULL);
    assert(cell->type == SC_CELL_STRING);
    assert(strcmp(cell->value.string, "Hello") == 0);
    sc_cell_destroy(cell);
    printf("  PASS: create string\n");
}

static void test_create_boolean(void) {
    SCCell *cell = sc_cell_create_boolean(true);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_BOOLEAN);
    assert(cell->value.boolean == true);
    sc_cell_destroy(cell);
    printf("  PASS: create boolean\n");
}

static void test_create_error(void) {
    SCCell *cell = sc_cell_create_error(SC_ERROR_DIV0);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_ERROR);
    assert(cell->value.error == SC_ERROR_DIV0);
    assert(strcmp(sc_error_to_string(SC_ERROR_DIV0), "#DIV/0!") == 0);
    sc_cell_destroy(cell);
    printf("  PASS: create error\n");
}

static void test_create_formula(void) {
    SCCell *cell = sc_cell_create_formula("=A1+B1", 15.0);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_FORMULA);
    assert(strcmp(cell->formula_text, "=A1+B1") == 0);
    assert(cell->computed_value == 15.0);
    sc_cell_destroy(cell);
    printf("  PASS: create formula\n");
}

static void test_create_empty(void) {
    SCCell *cell = sc_cell_create_empty();
    assert(cell != NULL);
    assert(cell->type == SC_CELL_EMPTY);
    sc_cell_destroy(cell);
    printf("  PASS: create empty\n");
}

static void test_clone(void) {
    SCCell *orig = sc_cell_create_string("test");
    SCCellStyle *style = sc_cell_style_create();
    style->bold = true;
    style->font_size = 14;
    sc_cell_set_style(orig, style);

    SCCell *clone = sc_cell_clone(orig);
    assert(clone != NULL);
    assert(clone->type == SC_CELL_STRING);
    assert(strcmp(clone->value.string, "test") == 0);
    assert(clone->style != NULL);
    assert(clone->style->bold == true);
    assert(clone->style->font_size == 14);

    /* Verify deep copy (different pointers) */
    assert(clone->value.string != orig->value.string);
    assert(clone->style != orig->style);

    sc_cell_destroy(orig);
    sc_cell_destroy(clone);
    printf("  PASS: clone\n");
}

static void test_display_string(void) {
    char buf[256];

    SCCell *num = sc_cell_create_number(3.14);
    sc_cell_get_display_string(num, buf, sizeof(buf));
    assert(strcmp(buf, "3.14") == 0);
    sc_cell_destroy(num);

    SCCell *str = sc_cell_create_string("hello");
    sc_cell_get_display_string(str, buf, sizeof(buf));
    assert(strcmp(buf, "hello") == 0);
    sc_cell_destroy(str);

    SCCell *bl = sc_cell_create_boolean(false);
    sc_cell_get_display_string(bl, buf, sizeof(buf));
    assert(strcmp(buf, "FALSE") == 0);
    sc_cell_destroy(bl);

    SCCell *err = sc_cell_create_error(SC_ERROR_VALUE);
    sc_cell_get_display_string(err, buf, sizeof(buf));
    assert(strcmp(buf, "#VALUE!") == 0);
    sc_cell_destroy(err);

    printf("  PASS: display string\n");
}

static void test_style(void) {
    SCCellStyle *style = sc_cell_style_create();
    assert(style != NULL);
    assert(style->bold == false);
    assert(style->font_size == 0);

    style->bold = true;
    style->italic = true;
    style->font_color = 0xFF0000;
    style->number_format = strdup("#,##0.00");

    SCCellStyle *clone = sc_cell_style_clone(style);
    assert(clone->bold == true);
    assert(clone->italic == true);
    assert(clone->font_color == 0xFF0000);
    assert(strcmp(clone->number_format, "#,##0.00") == 0);
    assert(clone->number_format != style->number_format);

    sc_cell_style_destroy(style);
    sc_cell_style_destroy(clone);
    printf("  PASS: style\n");
}

static void test_cellkey(void) {
    SCCellKey k1 = sc_cellkey_make(0, 0);
    SCCellKey k2 = sc_cellkey_make(0, 0);
    SCCellKey k3 = sc_cellkey_make(1, 0);

    assert(sc_cellkey_equal(k1, k2));
    assert(!sc_cellkey_equal(k1, k3));

    uint64_t p1 = sc_cellkey_pack(k1);
    uint64_t p3 = sc_cellkey_pack(k3);
    assert(p1 != p3);

    printf("  PASS: cellkey\n");
}

static void test_error_strings(void) {
    assert(strcmp(sc_error_to_string(SC_ERROR_DIV0), "#DIV/0!") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_VALUE), "#VALUE!") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_REF), "#REF!") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_NAME), "#NAME?") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_NUM), "#NUM!") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_NA), "#N/A") == 0);
    assert(strcmp(sc_error_to_string(SC_ERROR_NULL), "#NULL!") == 0);
    printf("  PASS: error strings\n");
}

int main(void) {
    printf("test_cell:\n");
    test_create_number();
    test_create_string();
    test_create_boolean();
    test_create_error();
    test_create_formula();
    test_create_empty();
    test_clone();
    test_display_string();
    test_style();
    test_cellkey();
    test_error_strings();
    printf("All cell tests passed!\n");
    return 0;
}
