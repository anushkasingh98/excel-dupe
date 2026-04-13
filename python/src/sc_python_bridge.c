/* Phase 6: Python integration bridge
 * Placeholder — will embed CPython and register the sheetcore module */

#include "sc_python_bridge.h"
#include <stdio.h>

int sc_python_init(SCWorkbook *wb) {
    (void)wb;
    fprintf(stderr, "Python integration not yet implemented (Phase 6)\n");
    return -1;
}

void sc_python_finalize(void) {
    /* no-op */
}

int sc_python_exec(const char *script) {
    (void)script;
    return -1;
}

int sc_python_exec_file(const char *path) {
    (void)path;
    return -1;
}
