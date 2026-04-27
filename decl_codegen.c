#include "decl.h"
#include "stmt.h"
#include "expr.h"
#include "symbol.h"
#include <stdio.h>

void decl_codegen(struct decl *d, FILE *out) {
    if (!d) return;

    if (d->type->kind == TYPE_FUNCTION) {
        if (d->code) {
            fprintf(out, "    .text\n");
            fprintf(out, "    .globl %s\n", d->name);
            fprintf(out, "%s:\n", d->name);

            fprintf(out, "    pushq %%rbp\n");
            fprintf(out, "    movq %%rsp, %%rbp\n");
            fprintf(out, "    subq $1032, %%rsp\n");


            // we stored number of locals in function symbol->which (see edits below)
            int nlocals = (d->symbol ? d->symbol->which : 0);
            if (nlocals > 0) {
                fprintf(out, "    subq $%d, %%rsp\n", nlocals * 8);
            }

            stmt_codegen(d->code, out);

            // default fall-through return 0
            fprintf(out, "    movq $0, %%rax\n");
            fprintf(out, "    leave\n");
            fprintf(out, "    ret\n");
        }
    } else {
        fprintf(out, "    .data\n");
        fprintf(out, "    .globl %s\n", d->name);
        fprintf(out, "%s:\n", d->name);

        if (d->value && d->value->kind == EXPR_INTEGER_LITERAL) {
            fprintf(out, "    .quad %d\n", d->value->literal_value);
        } else if (d->value && d->value->kind == EXPR_BOOL_LITERAL) {
            fprintf(out, "    .quad %d\n", d->value->literal_value ? 1 : 0);
        } else if (d->value && d->value->kind == EXPR_CHAR_LITERAL) {
            fprintf(out, "    .quad %d\n", d->value->literal_value);
        } else {
            fprintf(out, "    .quad 0\n");
        }
    }

    decl_codegen(d->next, out);
}
