#include "sc_engine.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

SCEngine *sc_engine_create(void) {
    SCEngine *engine = calloc(1, sizeof(SCEngine));
    if (!engine) return NULL;

    engine->workbook = sc_workbook_create();
    engine->selection = sc_selection_create();
    engine->undo_stack = sc_undo_stack_create();

    if (!engine->workbook || !engine->selection || !engine->undo_stack) {
        sc_engine_destroy(engine);
        return NULL;
    }
    return engine;
}

void sc_engine_destroy(SCEngine *engine) {
    if (!engine) return;
    sc_workbook_destroy(engine->workbook);
    sc_selection_destroy(engine->selection);
    sc_undo_stack_destroy(engine->undo_stack);
    free(engine);
}

SCSheet *sc_engine_active_sheet(const SCEngine *engine) {
    if (!engine) return NULL;
    return sc_workbook_get_sheet(engine->workbook, engine->workbook->active_sheet);
}

/* Parse user input and determine cell type */
static SCCell *parse_input(const char *input) {
    if (!input || *input == '\0') {
        return sc_cell_create_empty();
    }

    /* Formula */
    if (input[0] == '=') {
        return sc_cell_create_formula(input, 0.0);
    }

    /* Boolean */
    if (strcasecmp(input, "TRUE") == 0) {
        return sc_cell_create_boolean(true);
    }
    if (strcasecmp(input, "FALSE") == 0) {
        return sc_cell_create_boolean(false);
    }

    /* Try number */
    char *end;
    double num = strtod(input, &end);
    if (end != input && *end == '\0') {
        return sc_cell_create_number(num);
    }

    /* Default: string */
    return sc_cell_create_string(input);
}

void sc_engine_set_cell_value(SCEngine *engine, uint32_t row, uint16_t col,
                              const char *input) {
    if (!engine) return;

    SCSheet *sheet = sc_engine_active_sheet(engine);
    if (!sheet) return;

    int sheet_idx = engine->workbook->active_sheet;

    /* Capture old cell for undo */
    SCCell *old_cell = sc_sheet_get_cell(sheet, row, col);
    SCCell *new_cell = parse_input(input);

    /* Create undo action */
    SCUndoAction *action = sc_undo_action_create("Edit cell", sheet_idx);
    sc_undo_action_add_change(action, sc_cellkey_make(row, col), old_cell, new_cell);
    sc_undo_push(engine->undo_stack, action);

    /* Apply the change */
    if (new_cell->type == SC_CELL_EMPTY) {
        sc_sheet_remove_cell(sheet, row, col);
        sc_cell_destroy(new_cell);
    } else {
        sc_sheet_set_cell(sheet, row, col, new_cell);
    }
}

void sc_engine_clear_selection(SCEngine *engine) {
    if (!engine) return;

    SCSheet *sheet = sc_engine_active_sheet(engine);
    if (!sheet) return;

    int sheet_idx = engine->workbook->active_sheet;
    SCUndoAction *action = sc_undo_action_create("Clear cells", sheet_idx);

    for (int r = 0; r < engine->selection->range_count; r++) {
        SCRange range = sc_range_normalize(engine->selection->ranges[r]);
        for (uint32_t row = range.start_row; row <= range.end_row; row++) {
            for (uint16_t col = range.start_col; col <= range.end_col; col++) {
                SCCell *old_cell = sc_sheet_get_cell(sheet, row, col);
                if (old_cell) {
                    sc_undo_action_add_change(action, sc_cellkey_make(row, col),
                                              old_cell, NULL);
                    sc_sheet_remove_cell(sheet, row, col);
                }
            }
        }
    }

    sc_undo_push(engine->undo_stack, action);
}

void sc_engine_delete_cell(SCEngine *engine, uint32_t row, uint16_t col) {
    if (!engine) return;

    SCSheet *sheet = sc_engine_active_sheet(engine);
    if (!sheet) return;

    SCCell *old_cell = sc_sheet_get_cell(sheet, row, col);
    if (!old_cell) return;

    int sheet_idx = engine->workbook->active_sheet;
    SCUndoAction *action = sc_undo_action_create("Delete cell", sheet_idx);
    sc_undo_action_add_change(action, sc_cellkey_make(row, col), old_cell, NULL);
    sc_undo_push(engine->undo_stack, action);

    sc_sheet_remove_cell(sheet, row, col);
}

bool sc_engine_undo(SCEngine *engine) {
    if (!engine) return false;

    SCUndoAction *action = sc_undo_undo(engine->undo_stack);
    if (!action) return false;

    SCSheet *sheet = sc_workbook_get_sheet(engine->workbook, action->sheet_index);
    if (!sheet) return false;

    /* Apply old cell values */
    for (int i = 0; i < action->change_count; i++) {
        SCCellChange *change = &action->changes[i];
        if (change->old_cell) {
            sc_sheet_set_cell(sheet, change->key.row, change->key.col,
                              sc_cell_clone(change->old_cell));
        } else {
            sc_sheet_remove_cell(sheet, change->key.row, change->key.col);
        }
    }
    return true;
}

bool sc_engine_redo(SCEngine *engine) {
    if (!engine) return false;

    SCUndoAction *action = sc_undo_redo(engine->undo_stack);
    if (!action) return false;

    SCSheet *sheet = sc_workbook_get_sheet(engine->workbook, action->sheet_index);
    if (!sheet) return false;

    /* Apply new cell values */
    for (int i = 0; i < action->change_count; i++) {
        SCCellChange *change = &action->changes[i];
        if (change->new_cell) {
            sc_sheet_set_cell(sheet, change->key.row, change->key.col,
                              sc_cell_clone(change->new_cell));
        } else {
            sc_sheet_remove_cell(sheet, change->key.row, change->key.col);
        }
    }
    return true;
}
