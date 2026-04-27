#include "scratch.h"
#include <stdio.h>
#include <stdlib.h>

static int scratch_inuse[6] = {0};

int scratch_alloc(void) {
    for (int i = 0; i < 6; i++) {
        if (!scratch_inuse[i]) {
            scratch_inuse[i] = 1;
            return i;
        }
    }
    fprintf(stderr, "codegen error: out of scratch registers\n");
    exit(1);
}

void scratch_free(int r) {
    if (r < 0 || r >= 6) return;
    scratch_inuse[r] = 0;
}

const char *scratch_name(int r) {
    static const char *names[6] = {"%r10","%r11","%r12","%r13","%r14","%r15"};
    if (r < 0 || r >= 6) return "%r10";
    return names[r];
}
