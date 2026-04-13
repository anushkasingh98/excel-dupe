#include "sc_undo.h"
#include <stdlib.h>
#include <string.h>

SCUndoStack *sc_undo_stack_create(void) {
    SCUndoStack *stack = calloc(1, sizeof(SCUndoStack));
    return stack;
}

static void undo_action_free(SCUndoAction *action) {
    if (!action) return;
    free(action->description);
    for (int i = 0; i < action->change_count; i++) {
        sc_cell_destroy(action->changes[i].old_cell);
        sc_cell_destroy(action->changes[i].new_cell);
    }
    free(action->changes);
    free(action);
}

void sc_undo_stack_destroy(SCUndoStack *stack) {
    if (!stack) return;
    for (int i = 0; i < stack->count; i++) {
        undo_action_free(stack->actions[i]);
    }
    free(stack);
}

SCUndoAction *sc_undo_action_create(const char *description, int sheet_index) {
    SCUndoAction *action = calloc(1, sizeof(SCUndoAction));
    if (!action) return NULL;
    action->description = description ? strdup(description) : NULL;
    action->sheet_index = sheet_index;
    return action;
}

void sc_undo_action_add_change(SCUndoAction *action, SCCellKey key,
                               SCCell *old_cell, SCCell *new_cell) {
    if (!action) return;

    int new_count = action->change_count + 1;
    SCCellChange *new_changes = realloc(action->changes,
                                        (size_t)new_count * sizeof(SCCellChange));
    if (!new_changes) return;

    new_changes[action->change_count] = (SCCellChange){
        .key = key,
        .old_cell = old_cell ? sc_cell_clone(old_cell) : NULL,
        .new_cell = new_cell ? sc_cell_clone(new_cell) : NULL,
    };

    action->changes = new_changes;
    action->change_count = new_count;
}

void sc_undo_action_destroy(SCUndoAction *action) {
    undo_action_free(action);
}

void sc_undo_push(SCUndoStack *stack, SCUndoAction *action) {
    if (!stack || !action) return;

    /* Clear any redo actions (actions after current) */
    for (int i = stack->current; i < stack->count; i++) {
        undo_action_free(stack->actions[i]);
        stack->actions[i] = NULL;
    }
    stack->count = stack->current;

    /* If at capacity, remove oldest action */
    if (stack->count >= SC_UNDO_MAX_STACK) {
        undo_action_free(stack->actions[0]);
        memmove(&stack->actions[0], &stack->actions[1],
                (size_t)(stack->count - 1) * sizeof(SCUndoAction *));
        stack->count--;
        stack->current--;
    }

    stack->actions[stack->count] = action;
    stack->count++;
    stack->current = stack->count;
}

bool sc_undo_can_undo(const SCUndoStack *stack) {
    return stack && stack->current > 0;
}

bool sc_undo_can_redo(const SCUndoStack *stack) {
    return stack && stack->current < stack->count;
}

SCUndoAction *sc_undo_undo(SCUndoStack *stack) {
    if (!sc_undo_can_undo(stack)) return NULL;
    stack->current--;
    return stack->actions[stack->current];
}

SCUndoAction *sc_undo_redo(SCUndoStack *stack) {
    if (!sc_undo_can_redo(stack)) return NULL;
    SCUndoAction *action = stack->actions[stack->current];
    stack->current++;
    return action;
}

void sc_undo_clear(SCUndoStack *stack) {
    if (!stack) return;
    for (int i = 0; i < stack->count; i++) {
        undo_action_free(stack->actions[i]);
        stack->actions[i] = NULL;
    }
    stack->count = 0;
    stack->current = 0;
}
