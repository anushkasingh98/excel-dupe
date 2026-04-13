#ifndef SC_WORKBOOK_H
#define SC_WORKBOOK_H

#include "sc_sheet.h"

#define SC_MAX_SHEETS 256

typedef struct SCWorkbook {
    SCSheet *sheets[SC_MAX_SHEETS];
    int sheet_count;
    int active_sheet;
} SCWorkbook;

/* Create/destroy */
SCWorkbook *sc_workbook_create(void);
void sc_workbook_destroy(SCWorkbook *wb);

/* Sheet management */
SCSheet *sc_workbook_add_sheet(SCWorkbook *wb, const char *name);
bool sc_workbook_remove_sheet(SCWorkbook *wb, int index);
bool sc_workbook_rename_sheet(SCWorkbook *wb, int index, const char *new_name);
bool sc_workbook_move_sheet(SCWorkbook *wb, int from_index, int to_index);
SCSheet *sc_workbook_duplicate_sheet(SCWorkbook *wb, int index, const char *new_name);

/* Accessors */
SCSheet *sc_workbook_get_sheet(const SCWorkbook *wb, int index);
SCSheet *sc_workbook_get_sheet_by_name(const SCWorkbook *wb, const char *name);
int sc_workbook_get_sheet_index(const SCWorkbook *wb, const char *name);
int sc_workbook_get_active_sheet(const SCWorkbook *wb);
void sc_workbook_set_active_sheet(SCWorkbook *wb, int index);

#endif /* SC_WORKBOOK_H */
