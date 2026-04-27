#include "stmt.h"
#include "scope.h"
#include "type.h"

struct stmt * stmt_create( stmt_t kind, struct decl *decl, struct expr *init_expr, struct expr 
	*expr, struct expr *next_expr, struct stmt *body, struct stmt *else_body, struct stmt *next ) {
		
	struct stmt *s = (struct stmt *)malloc(sizeof(*s));
	if (!s) {
		fprintf(stderr, "memory allocation failed: stmt_create()");
		exit(1);
	}

	s->kind = kind;
	s->decl = decl;
	s->init_expr = init_expr;
	s->expr = expr;
	s->next_expr = next_expr;
	s->body = body;
	s->else_body = else_body;
	s->next = next;

	return s;
}

static void Indent_print(int level) {
    for(int i = 0; i<level; i++){
                printf("    ");
            }
    return;
}

//had to edit what we turned in for ICE, wasn't printing properly
void stmt_print( struct stmt *s, int indent ) {
    if (!s) {
        fprintf(stderr, "\nstmt_print() failed\n");
        return;
    }

    switch (s->kind) {
        case STMT_DECL:
            decl_print(s->decl, indent);
            break;
        case STMT_EXPR:
            Indent_print(indent);
            expr_print(s->expr);
            printf(";\n");
            break;
        case STMT_IF_ELSE:
            Indent_print(indent);
            printf("if(");
            if (s->expr) {
                expr_print(s->expr);
            }
                printf(") ");

            if (s->body && s->body->kind == STMT_BLOCK) {
                stmt_print(s->body, indent);
            } else {
                printf("\n");
                stmt_print(s->body, indent + 1);
            }

            if (s->else_body) {
                Indent_print(indent);
                printf("else ");
                if (s->else_body->kind == STMT_BLOCK) {
                    stmt_print(s->else_body, indent);
                } else {
                    printf("\n");
                    stmt_print(s->else_body, indent + 1);
                }
            }
            break;
        case STMT_FOR:
            Indent_print(indent);
            printf("for(");

            if (s->init_expr) {
                expr_print(s->init_expr);
            }
           printf("; ");

            if (s->expr) {
                expr_print(s->expr);
            }
            printf("; ");

            if (s->next_expr) {
                expr_print(s->next_expr);
            }
            printf(") ");

            if (s->body && s->body->kind == STMT_BLOCK) {
                stmt_print(s->body, indent);
            } else {
                printf("\n");
                stmt_print(s->body, indent + 1);
            }
            break;
        case STMT_WHILE:
            Indent_print(indent);
            printf("while(");
            if (s->expr) {
                expr_print(s->expr);
            }
            printf(") ");

            if (s->body && s->body->kind == STMT_BLOCK) {
                stmt_print(s->body, indent);
            } else {
                printf("\n");
                stmt_print(s->body, indent + 1);
            }
            break;
        case STMT_PRINT:
            Indent_print(indent);
            printf("print ");
            if (s->expr) {
                expr_print(s->expr);
            }
            printf(";\n");
            break;
        case STMT_RETURN:
            Indent_print(indent);
            printf("return");
            if (s->expr) {
                printf(" ");
                expr_print(s->expr);
            }
            printf(";\n");
            break;
        case STMT_BLOCK:
            printf("{\n");
            if (s->body) {
                stmt_print(s->body, indent + 1);
            }
            Indent_print(indent);
            printf("}\n");
            break;
        default:
        Indent_print(indent);
        fprintf(stderr, "\nstmt_print() failure\n");
    }

    if (s->next) {
        stmt_print(s->next, indent);
    }
}

void stmt_resolve( struct stmt *s ) {
	if (!s) return;

	switch (s->kind) {

		case STMT_DECL:
			if (s->decl) { decl_resolve(s->decl); }
			break;

		case STMT_EXPR:
			if (s->expr) { expr_resolve(s->expr); }
			break;

		case STMT_IF_ELSE:
			if (s->expr) { expr_resolve(s->expr); }
			if (s->body) { stmt_resolve(s->body); }
			if (s->else_body) { stmt_resolve(s->else_body); }
			break;

		case STMT_FOR:
			if (s->init_expr) { expr_resolve(s->init_expr); }
			if (s->expr) { expr_resolve(s->expr); }
			if (s->next_expr) { expr_resolve(s->next_expr); }
			if (s->body) { stmt_resolve(s->body); }
			break;

		case STMT_WHILE:
			if (s->expr) { expr_resolve(s->expr); }
			if (s->body) { stmt_resolve(s->body); }
			break;

		case STMT_PRINT:
			if (s->expr) { expr_resolve(s->expr); }
			break;

		case STMT_RETURN:
			if (s->expr) { expr_resolve(s->expr); }
			break;

		case STMT_BLOCK:
			scope_enter();
			if (s->body) { stmt_resolve(s->body); }
			scope_exit();
			break;

		default:
			break;
	}

	if (s->next) stmt_resolve(s->next);
}

void stmt_typecheck( struct stmt *s, struct type *return_type ) {
	if (!s) return;

	switch (s->kind) {
		case STMT_DECL:
			if (s->decl) { 
				decl_typecheck(s->decl);
			}
			break;
		case STMT_EXPR: {
			struct type *t = expr_typecheck(s->expr);
			if (t) type_delete(t);
			break;
		}
		case STMT_IF_ELSE: {
			struct type *t = expr_typecheck(s->expr);
			if (t && t->kind != TYPE_BOOLEAN) {
				fprintf(stderr, "type error: if condition must be boolean\n");
			}
			if (t) type_delete(t);

			stmt_typecheck(s->body, return_type);
			stmt_typecheck(s->else_body, return_type);
			break;
		}
		case STMT_WHILE: {
			struct type *t = expr_typecheck(s->expr);
			if (t && t->kind != TYPE_BOOLEAN) {
				fprintf(stderr, "type error: while condition must be boolean\n");
			}
			if (t) type_delete(t);

			stmt_typecheck(s->body, return_type);
			break;
		}
		case STMT_FOR: {
			if (s->init_expr) {
				struct type *t1 = expr_typecheck(s->init_expr);
				if (t1) type_delete(t1);
			}

			if (s->expr) {
				struct type *t2 = expr_typecheck(s->expr);
				if (t2 && t2->kind != TYPE_BOOLEAN) {
					fprintf(stderr, "type error: for condition must be boolean\n");
				}
				if (t2) type_delete(t2);
			}

			if (s->next_expr) {
				struct type *t3 = expr_typecheck(s->next_expr);
				if (t3) type_delete(t3);
			}

			stmt_typecheck(s->body, return_type);
			break;
		}
		case STMT_PRINT: {
			struct type *t = expr_typecheck(s->expr);
			if (t) {
				if (t->kind == TYPE_VOID || t->kind == TYPE_FUNCTION || t->kind == TYPE_ARRAY) {
					fprintf(stderr, "type error: cannot print that type\n");
				}
				type_delete(t);
			}
			break;
		}
		case STMT_RETURN: {
			if (!s->expr) {
				if (return_type && return_type->kind != TYPE_VOID) {
					fprintf(stderr, "type error: non-void function must return a value\n");
				}
				break;
			}

			struct type *t = expr_typecheck(s->expr);
			if (t && return_type) {
				if (!type_equals(t, return_type)) {
					fprintf(stderr, "type error: return type does not match function return type\n");
				}
			}
			if (t) type_delete(t);
			break;
		}

		case STMT_BLOCK:
			stmt_typecheck(s->body, return_type);
			break;

		default:
			break;
	}

	stmt_typecheck(s->next, return_type);
}