#include "stmt.h"
#include "expr.h"
#include "scratch.h"
#include "label.h"
#include <stdio.h>

static void codegen_print_list(struct expr *list, FILE *out) {
    // list is EXPR_LIST: left=item, right=next
    while (list) {
        struct expr *item = list->left;
        int r = expr_codegen(item, out);

        // baseline: print as integer-like
        fprintf(out, "    movq %s, %%rdi\n", scratch_name(r));
        fprintf(out, "    call print_integer\n");
        scratch_free(r);

        if (list->right) list = list->right;
        else break;
    }

    // newline
    fprintf(out, "    movq $10, %%rdi\n");
    fprintf(out, "    call print_character\n");
}

void stmt_codegen(struct stmt *s, FILE *out) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
            // locals handled by stack allocation in function prologue
            break;

        case STMT_EXPR: {
            int r = expr_codegen(s->expr, out);
            scratch_free(r);
            break;
        }

        case STMT_PRINT:
            if (s->expr) codegen_print_list(s->expr, out);
            else {
                fprintf(out, "    movq $10, %%rdi\n");
                fprintf(out, "    call print_character\n");
            }
            break;

        case STMT_RETURN:
            if (s->expr) {
                int r = expr_codegen(s->expr, out);
                fprintf(out, "    movq %s, %%rax\n", scratch_name(r));
                scratch_free(r);
            }
            fprintf(out, "    leave\n");
            fprintf(out, "    ret\n");
            break;

        case STMT_BLOCK:
            stmt_codegen(s->body, out);
            break;

        case STMT_IF_ELSE: {
            int else_l = label_create();
            int done_l = label_create();

            int r = expr_codegen(s->expr, out);
            fprintf(out, "    cmpq $0, %s\n", scratch_name(r));
            fprintf(out, "    je %s\n", label_name(else_l));
            scratch_free(r);

            stmt_codegen(s->body, out);
            fprintf(out, "    jmp %s\n", label_name(done_l));

            fprintf(out, "%s:\n", label_name(else_l));
            if (s->else_body) stmt_codegen(s->else_body, out);

            fprintf(out, "%s:\n", label_name(done_l));
            break;
        }

        case STMT_WHILE: {
            int start_l = label_create();
            int done_l  = label_create();

            fprintf(out, "%s:\n", label_name(start_l));
            int r = expr_codegen(s->expr, out);
            fprintf(out, "    cmpq $0, %s\n", scratch_name(r));
            fprintf(out, "    je %s\n", label_name(done_l));
            scratch_free(r);

            stmt_codegen(s->body, out);
            fprintf(out, "    jmp %s\n", label_name(start_l));
            fprintf(out, "%s:\n", label_name(done_l));
            break;
        }

        case STMT_FOR: {
            int start_l = label_create();
            int done_l  = label_create();

            if (s->init_expr) {
                int r0 = expr_codegen(s->init_expr, out);
                scratch_free(r0);
            }

            fprintf(out, "%s:\n", label_name(start_l));

            if (s->expr) {
                int r1 = expr_codegen(s->expr, out);
                fprintf(out, "    cmpq $0, %s\n", scratch_name(r1));
                fprintf(out, "    je %s\n", label_name(done_l));
                scratch_free(r1);
            }

            stmt_codegen(s->body, out);

            if (s->next_expr) {
                int r2 = expr_codegen(s->next_expr, out);
                scratch_free(r2);
            }

            fprintf(out, "    jmp %s\n", label_name(start_l));
            fprintf(out, "%s:\n", label_name(done_l));
            break;
        }

        default:
            fprintf(stderr, "codegen: stmt kind %d not implemented\n", s->kind);
            break;
    }

    stmt_codegen(s->next, out);
}
