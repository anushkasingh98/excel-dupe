#ifndef SC_PYTHON_BRIDGE_H
#define SC_PYTHON_BRIDGE_H

#include "sc_workbook.h"

/* Initialize embedded Python interpreter */
int sc_python_init(SCWorkbook *wb);

/* Finalize Python interpreter */
void sc_python_finalize(void);

/* Execute a Python script string */
int sc_python_exec(const char *script);

/* Execute a Python script file */
int sc_python_exec_file(const char *path);

#endif /* SC_PYTHON_BRIDGE_H */
