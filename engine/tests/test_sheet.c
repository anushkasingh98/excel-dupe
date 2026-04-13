#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sc_sheet.h"
#include "sc_workbook.h"

static void test_sheet_create(void) {
    SCSheet *sheet = sc_sheet_create("Test");
    assert(sheet != NULL);
    assert(strcmp(sheet->name, "Test") == 0);
    assert(sc_sheet_cell_count(sheet) == 0);
    sc_sheet_destroy(sheet);
    printf("  PASS: sheet create\n");
}

static void test_set_get_cell(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");

    sc_sheet_set_number(sheet, 0, 0, 42.0);
    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_NUMBER);
    assert(cell->value.number == 42.0);

    /* Empty cell returns NULL */
    assert(sc_sheet_get_cell(sheet, 1, 1) == NULL);

    sc_sheet_destroy(sheet);
    printf("  PASS: set/get cell\n");
}

static void test_set_string(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");
    sc_sheet_set_string(sheet, 5, 3, "Hello World");

    SCCell *cell = sc_sheet_get_cell(sheet, 5, 3);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_STRING);
    assert(strcmp(cell->value.string, "Hello World") == 0);

    sc_sheet_destroy(sheet);
    printf("  PASS: set string\n");
}

static void test_remove_cell(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");
    sc_sheet_set_number(sheet, 0, 0, 10.0);
    assert(sc_sheet_cell_count(sheet) == 1);

    sc_sheet_remove_cell(sheet, 0, 0);
    assert(sc_sheet_cell_count(sheet) == 0);
    assert(sc_sheet_get_cell(sheet, 0, 0) == NULL);

    sc_sheet_destroy(sheet);
    printf("  PASS: remove cell\n");
}

static void test_overwrite_cell(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");
    sc_sheet_set_number(sheet, 0, 0, 10.0);
    sc_sheet_set_number(sheet, 0, 0, 20.0);
    assert(sc_sheet_cell_count(sheet) == 1);

    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->value.number == 20.0);

    sc_sheet_destroy(sheet);
    printf("  PASS: overwrite cell\n");
}

static void test_col_width(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");

    /* Default width */
    assert(sc_sheet_get_col_width(sheet, 0) == SC_DEFAULT_COL_WIDTH);

    sc_sheet_set_col_width(sheet, 0, 150.0);
    assert(sc_sheet_get_col_width(sheet, 0) == 150.0);

    /* Other columns still default */
    assert(sc_sheet_get_col_width(sheet, 1) == SC_DEFAULT_COL_WIDTH);

    sc_sheet_destroy(sheet);
    printf("  PASS: column width\n");
}

static void test_row_height(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");
    assert(sc_sheet_get_row_height(sheet, 0) == SC_DEFAULT_ROW_HEIGHT);

    sc_sheet_set_row_height(sheet, 0, 30.0);
    assert(sc_sheet_get_row_height(sheet, 0) == 30.0);

    sc_sheet_destroy(sheet);
    printf("  PASS: row height\n");
}

static void test_used_range(void) {
    SCSheet *sheet = sc_sheet_create("Sheet1");

    /* Empty sheet has no used range */
    assert(!sc_sheet_get_used_range(sheet, NULL, NULL, NULL, NULL));

    sc_sheet_set_number(sheet, 2, 1, 10.0);  /* B3 */
    sc_sheet_set_number(sheet, 5, 3, 20.0);  /* D6 */

    uint32_t rmin, rmax;
    uint16_t cmin, cmax;
    assert(sc_sheet_get_used_range(sheet, &rmin, &cmin, &rmax, &cmax));
    assert(rmin == 2);
    assert(cmin == 1);
    assert(rmax == 5);
    assert(cmax == 3);

    sc_sheet_destroy(sheet);
    printf("  PASS: used range\n");
}

static void test_col_to_letter(void) {
    char buf[8];

    sc_col_to_letter(0, buf, sizeof(buf));
    assert(strcmp(buf, "A") == 0);

    sc_col_to_letter(25, buf, sizeof(buf));
    assert(strcmp(buf, "Z") == 0);

    sc_col_to_letter(26, buf, sizeof(buf));
    assert(strcmp(buf, "AA") == 0);

    sc_col_to_letter(27, buf, sizeof(buf));
    assert(strcmp(buf, "AB") == 0);

    sc_col_to_letter(701, buf, sizeof(buf));
    assert(strcmp(buf, "ZZ") == 0);

    sc_col_to_letter(702, buf, sizeof(buf));
    assert(strcmp(buf, "AAA") == 0);

    printf("  PASS: col to letter\n");
}

static void test_letter_to_col(void) {
    assert(sc_letter_to_col("A") == 0);
    assert(sc_letter_to_col("Z") == 25);
    assert(sc_letter_to_col("AA") == 26);
    assert(sc_letter_to_col("AB") == 27);
    assert(sc_letter_to_col("ZZ") == 701);
    assert(sc_letter_to_col("AAA") == 702);
    printf("  PASS: letter to col\n");
}

static void test_cellref_conversion(void) {
    char buf[16];
    SCCellKey key;

    sc_cellkey_to_ref(sc_cellkey_make(0, 0), buf, sizeof(buf));
    assert(strcmp(buf, "A1") == 0);

    sc_cellkey_to_ref(sc_cellkey_make(9, 2), buf, sizeof(buf));
    assert(strcmp(buf, "C10") == 0);

    assert(sc_ref_to_cellkey("A1", &key));
    assert(key.row == 0 && key.col == 0);

    assert(sc_ref_to_cellkey("C10", &key));
    assert(key.row == 9 && key.col == 2);

    assert(sc_ref_to_cellkey("$A$1", &key));
    assert(key.row == 0 && key.col == 0);

    assert(sc_ref_to_cellkey("AA100", &key));
    assert(key.row == 99 && key.col == 26);

    /* Invalid */
    assert(!sc_ref_to_cellkey("", &key));
    assert(!sc_ref_to_cellkey("123", &key));

    printf("  PASS: cellref conversion\n");
}

static void test_workbook_basic(void) {
    SCWorkbook *wb = sc_workbook_create();
    assert(wb != NULL);
    assert(wb->sheet_count == 1);
    assert(sc_workbook_get_sheet(wb, 0) != NULL);
    assert(strcmp(sc_workbook_get_sheet(wb, 0)->name, "Sheet1") == 0);

    /* Add sheet */
    SCSheet *s2 = sc_workbook_add_sheet(wb, "Data");
    assert(s2 != NULL);
    assert(wb->sheet_count == 2);
    assert(strcmp(s2->name, "Data") == 0);

    /* Get by name */
    assert(sc_workbook_get_sheet_by_name(wb, "Data") == s2);
    assert(sc_workbook_get_sheet_index(wb, "Data") == 1);
    assert(sc_workbook_get_sheet_by_name(wb, "nonexistent") == NULL);

    /* Remove */
    assert(sc_workbook_remove_sheet(wb, 0));
    assert(wb->sheet_count == 1);
    assert(strcmp(sc_workbook_get_sheet(wb, 0)->name, "Data") == 0);

    /* Can't remove last sheet */
    assert(!sc_workbook_remove_sheet(wb, 0));

    sc_workbook_destroy(wb);
    printf("  PASS: workbook basic\n");
}

static void test_workbook_rename(void) {
    SCWorkbook *wb = sc_workbook_create();
    assert(sc_workbook_rename_sheet(wb, 0, "MySheet"));
    assert(strcmp(sc_workbook_get_sheet(wb, 0)->name, "MySheet") == 0);
    sc_workbook_destroy(wb);
    printf("  PASS: workbook rename\n");
}

static void test_workbook_move(void) {
    SCWorkbook *wb = sc_workbook_create();
    sc_workbook_add_sheet(wb, "B");
    sc_workbook_add_sheet(wb, "C");
    /* Order: Sheet1, B, C */

    assert(sc_workbook_move_sheet(wb, 2, 0));
    /* Order: C, Sheet1, B */
    assert(strcmp(sc_workbook_get_sheet(wb, 0)->name, "C") == 0);
    assert(strcmp(sc_workbook_get_sheet(wb, 1)->name, "Sheet1") == 0);
    assert(strcmp(sc_workbook_get_sheet(wb, 2)->name, "B") == 0);

    sc_workbook_destroy(wb);
    printf("  PASS: workbook move\n");
}

static void test_workbook_duplicate(void) {
    SCWorkbook *wb = sc_workbook_create();
    SCSheet *s1 = sc_workbook_get_sheet(wb, 0);
    sc_sheet_set_number(s1, 0, 0, 99.0);

    SCSheet *dup = sc_workbook_duplicate_sheet(wb, 0, "Sheet1 Copy");
    assert(dup != NULL);
    assert(wb->sheet_count == 2);

    SCCell *cell = sc_sheet_get_cell(dup, 0, 0);
    assert(cell != NULL);
    assert(cell->value.number == 99.0);

    sc_workbook_destroy(wb);
    printf("  PASS: workbook duplicate\n");
}

static void test_many_cells(void) {
    SCSheet *sheet = sc_sheet_create("Big");

    /* Insert 10000 cells */
    for (int i = 0; i < 10000; i++) {
        sc_sheet_set_number(sheet, (uint32_t)i, 0, (double)i);
    }
    assert(sc_sheet_cell_count(sheet) == 10000);

    /* Verify some */
    SCCell *cell = sc_sheet_get_cell(sheet, 5000, 0);
    assert(cell != NULL);
    assert(cell->value.number == 5000.0);

    sc_sheet_destroy(sheet);
    printf("  PASS: many cells (10000)\n");
}

int main(void) {
    printf("test_sheet:\n");
    test_sheet_create();
    test_set_get_cell();
    test_set_string();
    test_remove_cell();
    test_overwrite_cell();
    test_col_width();
    test_row_height();
    test_used_range();
    test_col_to_letter();
    test_letter_to_col();
    test_cellref_conversion();
    test_workbook_basic();
    test_workbook_rename();
    test_workbook_move();
    test_workbook_duplicate();
    test_many_cells();
    printf("All sheet tests passed!\n");
    return 0;
}
