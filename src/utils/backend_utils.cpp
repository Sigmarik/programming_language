#include "frontend_utils.h"

#include <stdlib.h>
#include <stdarg.h>

#include "lib/util/dbg/logger.h"
#include "lib/util/dbg/debug.h"

#include "lib/speaker.h"

void print_label() {
    printf("Tree compiler by Ilya Kudryashov.\n");
    printf("Program transforms tree to working binary code.\n");
    printf("Build from\n%s %s\n", __DATE__, __TIME__);
    log_printf(ABSOLUTE_IMPORTANCE, "build info", "Build from %s %s.\n", __DATE__, __TIME__);
}
