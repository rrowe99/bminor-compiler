#include "label.h"
#include <stdio.h>

static int label_count = 0;

int label_create(void) {
    return label_count++;
}

const char *label_name(int label) {
    static char buf[64];
    snprintf(buf, sizeof(buf), ".L%d", label);
    return buf;
}
