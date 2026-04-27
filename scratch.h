#ifndef SCRATCH_H
#define SCRATCH_H

int scratch_alloc(void);
void scratch_free(int r);
const char *scratch_name(int r);

#endif
