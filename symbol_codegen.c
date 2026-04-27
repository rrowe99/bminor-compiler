#include "symbol_codegen.h"

void symbol_codegen(struct symbol *s, FILE *out) {
    if (!s) return;

    switch (s->kind) {
        case SYMBOL_GLOBAL:
            fprintf(out, "%s(%%rip)", s->name);
            break;
        case SYMBOL_LOCAL:
            fprintf(out, "-%d(%%rbp)", 8 * (s->which + 1));
            break;
        case SYMBOL_PARAM:
            fprintf(out, "%d(%%rbp)", 16 + 8 * s->which);
            break;
    }
}
