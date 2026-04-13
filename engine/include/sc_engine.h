#ifndef SC_ENGINE_H
#define SC_ENGINE_H

#include "sc_workbook.h"
#include "sc_selection.h"
#include "sc_undo.h"

/* Top-level engine facade: combines workbook, selection, and undo */
typedef struct {
    SCWorkbook *workbook;
    SCSelection *selection;
    SCUndoStack *undo_stack;
} SCEngine;

/* Create/destroy */
SCEngine *sc_engine_create(void);
void sc_engine_destroy(SCEngine *engine);

/* High-level operations (these record undo automatically) */
void sc_engine_set_cell_value(SCEngine *engine, uint32_t row, uint16_t col,
                              const char *input);
void sc_engine_clear_selection(SCEngine *engine);
void sc_engine_delete_cell(SCEngine *engine, uint32_t row, uint16_t col);

/* Undo/redo */
bool sc_engine_undo(SCEngine *engine);
bool sc_engine_redo(SCEngine *engine);

/* Convenience: get active sheet */
SCSheet *sc_engine_active_sheet(const SCEngine *engine);

#endif /* SC_ENGINE_H */
