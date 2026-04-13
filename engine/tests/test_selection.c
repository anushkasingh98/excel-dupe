#include <assert.h>
#include <stdio.h>
#include "sc_selection.h"

static void test_create(void) {
    SCSelection *sel = sc_selection_create();
    assert(sel != NULL);
    assert(sel->active_cell.row == 0);
    assert(sel->active_cell.col == 0);
    assert(sel->range_count == 1);
    sc_selection_destroy(sel);
    printf("  PASS: create\n");
}

static void test_set_cell(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_cell(sel, 5, 3);
    assert(sel->active_cell.row == 5);
    assert(sel->active_cell.col == 3);
    assert(sel->range_count == 1);
    assert(sel->ranges[0].start_row == 5);
    assert(sel->ranges[0].start_col == 3);
    assert(sel->ranges[0].end_row == 5);
    assert(sel->ranges[0].end_col == 3);
    sc_selection_destroy(sel);
    printf("  PASS: set cell\n");
}

static void test_set_range(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_range(sel, 1, 0, 5, 3);
    assert(sel->range_count == 1);
    assert(sc_selection_contains(sel, 1, 0));
    assert(sc_selection_contains(sel, 3, 2));
    assert(sc_selection_contains(sel, 5, 3));
    assert(!sc_selection_contains(sel, 0, 0));
    assert(!sc_selection_contains(sel, 6, 0));
    sc_selection_destroy(sel);
    printf("  PASS: set range\n");
}

static void test_add_range(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_cell(sel, 0, 0);
    sc_selection_add_range(sel, 5, 5, 7, 7);
    assert(sel->range_count == 2);
    assert(sc_selection_contains(sel, 0, 0));
    assert(sc_selection_contains(sel, 6, 6));
    assert(!sc_selection_contains(sel, 3, 3));
    sc_selection_destroy(sel);
    printf("  PASS: add range (multi-select)\n");
}

static void test_extend_to(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_cell(sel, 2, 2);
    sc_selection_extend_to(sel, 5, 5);
    assert(sel->range_count == 1);
    assert(sc_selection_contains(sel, 2, 2));
    assert(sc_selection_contains(sel, 5, 5));
    assert(sc_selection_contains(sel, 3, 4));
    sc_selection_destroy(sel);
    printf("  PASS: extend to\n");
}

static void test_move(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_cell(sel, 5, 5);

    sc_selection_move(sel, -1, 0);  /* up */
    assert(sel->active_cell.row == 4);
    assert(sel->active_cell.col == 5);

    sc_selection_move(sel, 0, 1);   /* right */
    assert(sel->active_cell.row == 4);
    assert(sel->active_cell.col == 6);

    sc_selection_move(sel, 1, 0);   /* down */
    assert(sel->active_cell.row == 5);

    sc_selection_move(sel, 0, -1);  /* left */
    assert(sel->active_cell.col == 5);

    sc_selection_destroy(sel);
    printf("  PASS: move\n");
}

static void test_move_bounds(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_cell(sel, 0, 0);

    /* Try to move up/left past origin */
    sc_selection_move(sel, -1, 0);
    assert(sel->active_cell.row == 0);
    sc_selection_move(sel, 0, -1);
    assert(sel->active_cell.col == 0);

    sc_selection_destroy(sel);
    printf("  PASS: move bounds\n");
}

static void test_range_normalize(void) {
    /* Backwards range */
    SCRange r = { .start_row = 5, .start_col = 3, .end_row = 1, .end_col = 0 };
    SCRange n = sc_range_normalize(r);
    assert(n.start_row == 1);
    assert(n.start_col == 0);
    assert(n.end_row == 5);
    assert(n.end_col == 3);
    printf("  PASS: range normalize\n");
}

static void test_range_cell_count(void) {
    SCRange r = { .start_row = 0, .start_col = 0, .end_row = 2, .end_col = 3 };
    assert(sc_range_cell_count(r) == 12); /* 3 rows x 4 cols */

    SCRange single = { .start_row = 5, .start_col = 5, .end_row = 5, .end_col = 5 };
    assert(sc_range_cell_count(single) == 1);
    printf("  PASS: range cell count\n");
}

static void test_bounding_range(void) {
    SCSelection *sel = sc_selection_create();
    sc_selection_set_range(sel, 2, 1, 4, 3);
    sc_selection_add_range(sel, 10, 0, 12, 5);

    SCRange bounds = sc_selection_get_bounding_range(sel);
    assert(bounds.start_row == 2);
    assert(bounds.start_col == 0);
    assert(bounds.end_row == 12);
    assert(bounds.end_col == 5);

    sc_selection_destroy(sel);
    printf("  PASS: bounding range\n");
}

int main(void) {
    printf("test_selection:\n");
    test_create();
    test_set_cell();
    test_set_range();
    test_add_range();
    test_extend_to();
    test_move();
    test_move_bounds();
    test_range_normalize();
    test_range_cell_count();
    test_bounding_range();
    printf("All selection tests passed!\n");
    return 0;
}
