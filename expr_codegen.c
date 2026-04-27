#include "expr.h"
#include "scratch.h"
#include "symbol_codegen.h"
#include <stdio.h>
#include <stdlib.h>

static void emit_call_args(FILE *out, struct expr *args) {
    const char *areg[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    int i = 0;

    struct expr *cur = args;

    while (cur && i < 6) {
        struct expr *a = cur;

        if (cur->kind == EXPR_LIST) {
            a = cur->left;
            cur = cur->right;
        } else {
            cur = NULL;
        }

        int r = expr_codegen(a, out);
        fprintf(out, "    movq %s, %s\n", scratch_name(r), areg[i]);
        scratch_free(r);
        i++;
    }
}


int expr_codegen(struct expr *e, FILE *out) {
    if (!e) return -1;

    switch (e->kind) {
        case EXPR_INTEGER_LITERAL: {
            int r = scratch_alloc();
            fprintf(out, "    movq $%d, %s\n", e->literal_value, scratch_name(r));
            return r;
        }
        
        case EXPR_BOOL_LITERAL: {
            int r = scratch_alloc();
            fprintf(out, "    movq $%d, %s\n", e->literal_value ? 1 : 0, scratch_name(r));
            return r;
        }

        case EXPR_CHAR_LITERAL: {
            int r = scratch_alloc();
            fprintf(out, "    movq $%d, %s\n", e->literal_value, scratch_name(r));
            return r;
        }

        case EXPR_STRING_LITERAL: {
            static int string_label = 0;
            int lbl = string_label++;

            fprintf(out, "    .data\n");
            fprintf(out, ".S%d:\n", lbl);
            fprintf(out, "    .string %s\n", e->string_literal);
            fprintf(out, "    .text\n");

            int r = scratch_alloc();
            fprintf(out, "    leaq .S%d(%%rip), %s\n", lbl, scratch_name(r));
            return r;
        }

        case EXPR_NAME: {
            int r = scratch_alloc();
            fprintf(out, "    movq ");
            symbol_codegen(e->symbol, out);
            fprintf(out, ", %s\n", scratch_name(r));
            return r;
        }

        case EXPR_ASSIGN: {
            int rr = expr_codegen(e->right, out);

            /* Only generate store if LHS is a plain variable */
            if (e->left && e->left->kind == EXPR_NAME && e->left->symbol) {
                fprintf(out, "    movq %s, ", scratch_name(rr));
                symbol_codegen(e->left->symbol, out);
                fprintf(out, "\n");
            }
            return rr;
        }


        case EXPR_ADD: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);
            fprintf(out, "    addq %s, %s\n", scratch_name(rr), scratch_name(rl));
            scratch_free(rr);
            return rl;
        }

        case EXPR_SUB: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);
            fprintf(out, "    subq %s, %s\n", scratch_name(rr), scratch_name(rl));
            scratch_free(rr);
            return rl;
        }

        case EXPR_MUL: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);
            fprintf(out, "    imulq %s, %s\n", scratch_name(rr), scratch_name(rl));
            scratch_free(rr);
            return rl;
        }

        case EXPR_DIV:
        case EXPR_EXP: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);
            fprintf(out, "    movq %s, %%rdi\n", scratch_name(rl));
            fprintf(out, "    movq %s, %%rsi\n", scratch_name(rr));
            fprintf(out, "    call integer_power\n");

            fprintf(out, "    movq %%rax, %s\n", scratch_name(rl));
            scratch_free(rr);
            
            return rl;
        }

        case EXPR_MOD: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);

            fprintf(out, "    movq %s, %%rax\n", scratch_name(rl));
            fprintf(out, "    cqto\n");
            fprintf(out, "    idivq %s\n", scratch_name(rr));

            if (e->kind == EXPR_DIV) {
                fprintf(out, "    movq %%rax, %s\n", scratch_name(rl));
            } else {
                fprintf(out, "    movq %%rdx, %s\n", scratch_name(rl));
            }

            scratch_free(rr);
            return rl;
        }

        case EXPR_EQ:
        case EXPR_NE:
        case EXPR_LT:
        case EXPR_LE:
        case EXPR_GT:
        case EXPR_GE: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);

            fprintf(out, "    cmpq %s, %s\n", scratch_name(rr), scratch_name(rl));

            const char *cc =
            e->kind == EXPR_EQ ? "sete" :
            e->kind == EXPR_NE ? "setne" :
            e->kind == EXPR_LT ? "setl" :
            e->kind == EXPR_LE ? "setle" :
            e->kind == EXPR_GT ? "setg" :
                                 "setge";

            fprintf(out, "    %s %%al\n", cc);
            fprintf(out, "    movzbq %%al, %s\n", scratch_name(rl));

            scratch_free(rr);
            return rl;
        }

        case EXPR_AND:
        case EXPR_OR: {
            int rl = expr_codegen(e->left, out);
            int rr = expr_codegen(e->right, out);
            
            if (e->kind == EXPR_AND)
            fprintf(out, "    andq %s, %s\n", scratch_name(rr), scratch_name(rl));
        else
        fprintf(out, "    orq %s, %s\n", scratch_name(rr), scratch_name(rl));

        fprintf(out, "    cmpq $0, %s\n", scratch_name(rl));
        fprintf(out, "    setne %%al\n");
        fprintf(out, "    movzbq %%al, %s\n", scratch_name(rl));

        scratch_free(rr);
        return rl;
    }
    
    case EXPR_NOT: {
        int r = expr_codegen(e->left, out);
        fprintf(out, "    cmpq $0, %s\n", scratch_name(r));
        fprintf(out, "    sete %%al\n");
        fprintf(out, "    movzbq %%al, %s\n", scratch_name(r));
        return r;
    }

    case EXPR_INC: {
        int r = scratch_alloc();
        fprintf(out, "    movq ");
        symbol_codegen(e->left->symbol, out);
        fprintf(out, ", %s\n", scratch_name(r));
        fprintf(out, "    addq $1, %s\n", scratch_name(r));
        fprintf(out, "    movq %s, ", scratch_name(r));
        symbol_codegen(e->left->symbol, out);
        fprintf(out, "\n");
        return r;
    }

    case EXPR_DEC: {
        int r = scratch_alloc();
        fprintf(out, "    movq ");
        symbol_codegen(e->left->symbol, out);
        fprintf(out, ", %s\n", scratch_name(r));
        fprintf(out, "    subq $1, %s\n", scratch_name(r));
        fprintf(out, "    movq %s, ", scratch_name(r));
        symbol_codegen(e->left->symbol, out);
        fprintf(out, "\n");
        return r;
    }

    case EXPR_FUNC_CALL: {
        if (e->left) {
            emit_call_args(out, e->left);
        }
        fprintf(out, "    call %s\n", e->name);
        int r = scratch_alloc();
        fprintf(out, "    movq %%rax, %s\n", scratch_name(r));
        return r;
    }


    case EXPR_LIST: {
        /* Evaluate left for side effects, ignore result */
        int r = expr_codegen(e->left, out);
        scratch_free(r);
        /* Continue list */
        if (e->right) {
            return expr_codegen(e->right, out);
        }
        
        return -1;
    }

    default:
        fprintf(stderr, "codegen: expr kind %d not implemented\n", e->kind);
        return -1;
}
}
