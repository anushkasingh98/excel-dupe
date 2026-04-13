#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sc_engine.h"

static void test_undo_basic(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    /* Set cell value */
    sc_engine_set_cell_value(engine, 0, 0, "42");

    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell != NULL);
    assert(cell->type == SC_CELL_NUMBER);
    assert(cell->value.number == 42.0);

    /* Undo */
    assert(sc_undo_can_undo(engine->undo_stack));
    assert(sc_engine_undo(engine));

    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell == NULL); /* should be back to empty */

    /* Redo */
    assert(sc_undo_can_redo(engine->undo_stack));
    assert(sc_engine_redo(engine));

    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell != NULL);
    assert(cell->value.number == 42.0);

    sc_engine_destroy(engine);
    printf("  PASS: undo/redo basic\n");
}

static void test_undo_multiple(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_engine_set_cell_value(engine, 0, 0, "10");
    sc_engine_set_cell_value(engine, 0, 0, "20");
    sc_engine_set_cell_value(engine, 0, 0, "30");

    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->value.number == 30.0);

    /* Undo 3 times */
    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell != NULL && cell->value.number == 20.0);

    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell != NULL && cell->value.number == 10.0);

    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell == NULL);

    /* No more undos */
    assert(!sc_engine_undo(engine));

    sc_engine_destroy(engine);
    printf("  PASS: undo multiple\n");
}

static void test_undo_clears_redo(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_engine_set_cell_value(engine, 0, 0, "10");
    sc_engine_set_cell_value(engine, 0, 0, "20");

    /* Undo once */
    assert(sc_engine_undo(engine));

    /* New action should clear redo stack */
    sc_engine_set_cell_value(engine, 0, 0, "99");
    assert(!sc_undo_can_redo(engine->undo_stack));

    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->value.number == 99.0);

    sc_engine_destroy(engine);
    printf("  PASS: undo clears redo on new action\n");
}

static void test_undo_string_cell(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_engine_set_cell_value(engine, 0, 0, "Hello");
    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->type == SC_CELL_STRING);
    assert(strcmp(cell->value.string, "Hello") == 0);

    sc_engine_set_cell_value(engine, 0, 0, "World");
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(strcmp(cell->value.string, "World") == 0);

    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(strcmp(cell->value.string, "Hello") == 0);

    sc_engine_destroy(engine);
    printf("  PASS: undo string cell\n");
}

static void test_undo_boolean(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_engine_set_cell_value(engine, 0, 0, "TRUE");
    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->type == SC_CELL_BOOLEAN);
    assert(cell->value.boolean == true);

    sc_engine_set_cell_value(engine, 0, 0, "FALSE");
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->value.boolean == false);

    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->value.boolean == true);

    sc_engine_destroy(engine);
    printf("  PASS: undo boolean\n");
}

static void test_undo_formula(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_engine_set_cell_value(engine, 0, 0, "=A2+1");
    SCCell *cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell->type == SC_CELL_FORMULA);
    assert(strcmp(cell->formula_text, "=A2+1") == 0);

    assert(sc_engine_undo(engine));
    cell = sc_sheet_get_cell(sheet, 0, 0);
    assert(cell == NULL);

    sc_engine_destroy(engine);
    printf("  PASS: undo formula\n");
}

static void test_clear_selection(void) {
    SCEngine *engine = sc_engine_create();
    SCSheet *sheet = sc_engine_active_sheet(engine);

    sc_sheet_set_number(sheet, 0, 0, 10.0);
    sc_sheet_set_number(sheet, 0, 1, 20.0);
    sc_sheet_set_number(sheet, 1, 0, 30.0);

    /* Select range A1:B2 */
    sc_selection_set_range(engine->selection, 0, 0, 1, 1);

    sc_engine_clear_selection(engine);

    assert(sc_sheet_get_cell(sheet, 0, 0) == NULL);
    assert(sc_sheet_get_cell(sheet, 0, 1) == NULL);
    assert(sc_sheet_get_cell(sheet, 1, 0) == NULL);

    /* Undo should restore all three cells */
    assert(sc_engine_undo(engine));
    assert(sc_sheet_get_cell(sheet, 0, 0) != NULL);
    assert(sc_sheet_get_cell(sheet, 0, 0)->value.number == 10.0);
    assert(sc_sheet_get_cell(sheet, 0, 1) != NULL);
    assert(sc_sheet_get_cell(sheet, 0, 1)->value.number == 20.0);
    assert(sc_sheet_get_cell(sheet, 1, 0) != NULL);
    assert(sc_sheet_get_cell(sheet, 1, 0)->value.number == 30.0);

    sc_engine_destroy(engine);
    printf("  PASS: clear selection + undo\n");
}

static void test_undo_stack_limit(void) {
    SCEngine *engine = sc_engine_create();

    /* Push more than SC_UNDO_MAX_STACK actions */
    for (int i = 0; i < SC_UNDO_MAX_STACK + 10; i++) {
        char val[16];
        snprintf(val, sizeof(val), "%d", i);
        sc_engine_set_cell_value(engine, 0, 0, val);
    }

    /* Stack should be at max */
    assert(engine->undo_stack->count <= SC_UNDO_MAX_STACK);

    sc_engine_destroy(engine);
    printf("  PASS: undo stack limit\n");
}

int main(void) {
    printf("test_undo:\n");
    test_undo_basic();
    test_undo_multiple();
    test_undo_clears_redo();
    test_undo_string_cell();
    test_undo_boolean();
    test_undo_formula();
    test_clear_selection();
    test_undo_stack_limit();
    printf("All undo tests passed!\n");
    return 0;
}
