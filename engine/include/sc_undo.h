#ifndef SC_UNDO_H
#define SC_UNDO_H

#include "sc_cell.h"
#include <stdbool.h>

#define SC_UNDO_MAX_STACK 100

/* A single cell change: stores the key and old + new cell data */
typedef struct {
    SCCellKey key;
    SCCell *old_cell;   /* NULL if cell was empty before */
    SCCell *new_cell;   /* NULL if cell was cleared */
} SCCellChange;

/* An undo action groups one or more cell changes */
typedef struct {
    char *description;
    SCCellChange *changes;
    int change_count;
    int sheet_index;
} SCUndoAction;

typedef struct {
    SCUndoAction *actions[SC_UNDO_MAX_STACK];
    int count;
    int current;   /* index of the next action to undo (points past last pushed) */
} SCUndoStack;

/* Create/destroy */
SCUndoStack *sc_undo_stack_create(void);
void sc_undo_stack_destroy(SCUndoStack *stack);

/* Action building */
SCUndoAction *sc_undo_action_create(const char *description, int sheet_index);
void sc_undo_action_add_change(SCUndoAction *action, SCCellKey key,
                               SCCell *old_cell, SCCell *new_cell);
void sc_undo_action_destroy(SCUndoAction *action);

/* Stack operations */
void sc_undo_push(SCUndoStack *stack, SCUndoAction *action);
bool sc_undo_can_undo(const SCUndoStack *stack);
bool sc_undo_can_redo(const SCUndoStack *stack);

/* Returns the action to apply (caller applies the changes). Does NOT free the action. */
SCUndoAction *sc_undo_undo(SCUndoStack *stack);
SCUndoAction *sc_undo_redo(SCUndoStack *stack);

/* Clear all history */
void sc_undo_clear(SCUndoStack *stack);

#endif /* SC_UNDO_H */
