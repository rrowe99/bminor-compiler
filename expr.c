#include <stdio.h>
#include "expr.h"
#include "scope.h"
#include "param_list.h"


struct expr * expr_create( expr_t kind, struct expr *left, struct expr *right ) {
	struct expr *e = (struct expr *)malloc(sizeof(*e));
	if (!e) {
		fprintf(stderr, "memory allocation failed: expr_create()");
		exit(1);
	}

	e->kind = kind;
	e->left = left;
	e->right = right;
	e->name = NULL;
	e->literal_value = 0;
	e->string_literal = NULL;
	e->symbol = NULL;

	return e;
}

struct expr * expr_create_name( const char *n ) {
	struct expr *e = expr_create(EXPR_NAME, NULL, NULL);
    e->name = n;

	return e;
}

struct expr * expr_create_integer_literal( int c ) {
	struct expr *e = expr_create(EXPR_INTEGER_LITERAL, NULL, NULL);
    e->literal_value = c;

	return e;
}

struct expr * expr_create_boolean_literal( int c ) {
	struct expr *e = expr_create(EXPR_BOOL_LITERAL, NULL, NULL);
    e->literal_value = c;

	return e;
}

struct expr * expr_create_char_literal( char c ) {
	struct expr *e = expr_create(EXPR_CHAR_LITERAL, NULL, NULL);
    e->literal_value = (int)c;

	return e;
}
struct expr * expr_create_string_literal( const char *str ) {
	struct expr *e = expr_create(EXPR_STRING_LITERAL, NULL, NULL);
    e->string_literal = str;

	return e;
}

void expr_print( struct expr *e ) {
    if (!e) {
        fprintf(stderr, "\nexpr_print() failed\n");
        return;
    };

    switch (e->kind) {
        case EXPR_ADD:
            printf("(");
            expr_print(e->left);
            printf(" + ");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_SUB:
            printf("(");
            expr_print(e->left);
            printf(" - ");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_MUL:
            printf("(");
            expr_print(e->left);
            printf(" * ");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_DIV:
            printf("(");
            expr_print(e->left);
            printf(" / ");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_NAME: 
            printf("%s", e->name);
            break;
        case EXPR_MOD:
            printf("(");
            expr_print(e->left);
            printf(" %% ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_EXP:
            printf("(");
            expr_print(e->left);
            printf(" ^ ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_EQ:
            printf("(");
            expr_print(e->left);
            printf(" == ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_NE:
            printf("(");
            expr_print(e->left);
            printf(" != ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_LT:
            printf("(");
            expr_print(e->left);
            printf(" < ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_LE:
            printf("(");
            expr_print(e->left);
            printf(" <= ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_GT:
            printf("(");
            expr_print(e->left);
            printf(" > ");
            expr_print(e->right);
            printf(")");
            break;
		case EXPR_GE:
            printf("(");
            expr_print(e->left);
            printf(" >= ");
            expr_print(e->right);
            printf(")");
            break;
        case EXPR_AND:
		    printf("(");
		    expr_print(e->left);
		    printf(" && ");
		    expr_print(e->right);
		    printf(")");
		    break;
        case EXPR_OR:
            printf("(");
            expr_print(e->left);
            printf(" || ");
            expr_print(e->right);
            printf(")");
		    break;
        case EXPR_NOT:
            printf("!");
            expr_print(e->left);
            break;
        case EXPR_ASSIGN:
            expr_print(e->left);
            printf(" = ");
            expr_print(e->right);
            break;
        case EXPR_CHAR_LITERAL:
            if (e->string_literal) {
                printf("%s", e->string_literal);
            } else {
                printf("'%c'", e->literal_value);
            }
            break;
        case EXPR_INTEGER_LITERAL:
            printf("%d", e->literal_value);
            break;
        case EXPR_STRING_LITERAL:
            printf("%s", e->string_literal);
            break;
		case EXPR_FUNC_CALL:
            printf("%s(", e->name);
		    if (e->left) {
			    expr_print(e->left);
		    }
            printf(")");
            break;
		case EXPR_ARR_ACC:  
            if (e->left) {
                if (e->left->kind == EXPR_NAME)
                    printf("%s[", e->left->name);
                else {
                    printf("(");
                    expr_print(e->left);
                    printf(")[");
                }
            }
            expr_print(e->right);
            printf("]");
            break;
        case EXPR_BOOL_LITERAL:
            printf(e->literal_value ? "true" : "false");
            break;
        case EXPR_INC:
            if (e->left->kind == EXPR_NAME) {
                printf("%s++", e->left->name);
            } else {
                printf("(");
                expr_print(e->left);
                printf(")++");
            }
            break;
        case EXPR_DEC:
            if (e->left->kind == EXPR_NAME) {
                printf("%s--", e->left->name);
            } else {
                printf("(");
                expr_print(e->left);
                printf(")--");
            }
            break;
        case EXPR_LIST:
	        expr_print(e->left);
	        if(e->right) {
		        printf(", ");
		        expr_print(e->right);
	        }
	        break;
        case EXPR_ARRAY_LIT:
            printf("{ ");
            if (e->left) {
                expr_print(e->left);
            }
            printf(" }");
            break;
        default:
			printf("unknown, expr_print()");
			break;
    }
}

void expr_resolve( struct expr *e ) {
	if (!e) { return; }

	if (e->left) { expr_resolve(e->left); }
	if (e->right) { expr_resolve(e->right); }

	if (e->kind == EXPR_NAME) {
		e->symbol = scope_lookup(e->name);
		if (!e->symbol) {
			fprintf(stderr, "resolve error: %s is not declared\n", e->name);
		}
		return;
	}

	if (e->kind == EXPR_FUNC_CALL) {
		e->symbol = scope_lookup(e->name);
		if (!e->symbol) {
			fprintf(stderr, "resolve error: %s is not declared\n", e->name);
		}
		return;
	}
}

struct type * expr_typecheck( struct expr *e ) {
	struct type *lt;
	struct type *rt;

	if (!e) return 0;

	switch (e->kind) {
		case EXPR_INTEGER_LITERAL:
			return type_create(TYPE_INTEGER, 0, 0);
		case EXPR_BOOL_LITERAL:
			return type_create(TYPE_BOOLEAN, 0, 0);
		case EXPR_CHAR_LITERAL:
			return type_create(TYPE_CHARACTER, 0, 0);

		case EXPR_STRING_LITERAL:
			return type_create(TYPE_STRING, 0, 0);

        case EXPR_NAME:
			if (!e->symbol) {
				fprintf(stderr, "type error: %s not resolved\n", e->name);
				return 0;
			}
			return type_copy(e->symbol->type);
        case EXPR_FUNC_CALL:
			if (!e->symbol) {
			fprintf(stderr, "type error: %s not resolved\n", e->name);
				return 0;
			}

			if (!e->symbol->type || e->symbol->type->kind != TYPE_FUNCTION) {
				fprintf(stderr, "type error: %s is not a function\n", e->name);
			return 0;
			}

			if (!param_list_typecheck(e->symbol->type->params, e->left)) {
				;//for validating arguments
			}

			return type_copy(e->symbol->type->subtype);	
        case EXPR_ARR_ACC:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (lt->kind != TYPE_ARRAY) {
				fprintf(stderr, "type error: cannot index non-array\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (rt->kind != TYPE_INTEGER) {
				fprintf(stderr, "type error: array index must be integer\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(rt);
			{
				struct type *result = type_copy(lt->subtype);
				type_delete(lt);
				return result;
			}
        case EXPR_NOT:
			lt = expr_typecheck(e->left);
			if (!lt) return 0;

			if (lt->kind != TYPE_BOOLEAN) {
				fprintf(stderr, "type error: ! requires boolean\n");
				type_delete(lt);
				return 0;
			}

			type_delete(lt);
			return type_create(TYPE_BOOLEAN, 0, 0);

		case EXPR_INC: 
		case EXPR_DEC:
			lt = expr_typecheck(e->left);
			if (!lt) return 0;

			if (lt->kind != TYPE_INTEGER) {
				fprintf(stderr, "type error: ++/-- requires integer\n");
				type_delete(lt);
				return 0;
			}

			type_delete(lt);
			return type_create(TYPE_INTEGER, 0, 0);
        case EXPR_ASSIGN:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (!type_equals(lt, rt)) {
				fprintf(stderr, "type error: assignment types do not match\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(rt);
			return lt;
        case EXPR_ADD:
		case EXPR_SUB:
		case EXPR_MUL:
		case EXPR_DIV:
		case EXPR_MOD:
		case EXPR_EXP:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER) {
				fprintf(stderr, "type error: arithmetic requires integers\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(lt);
			type_delete(rt);
			return type_create(TYPE_INTEGER, 0, 0);
		case EXPR_LT:
		case EXPR_LE:
		case EXPR_GT:
		case EXPR_GE:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER) {
				fprintf(stderr, "type error: comparison requires integers\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(lt);
			type_delete(rt);
			return type_create(TYPE_BOOLEAN, 0, 0);
		case EXPR_EQ:
		case EXPR_NE:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (!type_equals(lt, rt)) {
				fprintf(stderr, "type error: ==/!= requires matching types\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(lt);
			type_delete(rt);
			return type_create(TYPE_BOOLEAN, 0, 0);
		case EXPR_AND:
		case EXPR_OR:
			lt = expr_typecheck(e->left);
			rt = expr_typecheck(e->right);

			if (!lt || !rt) {
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			if (lt->kind != TYPE_BOOLEAN || rt->kind != TYPE_BOOLEAN) {
				fprintf(stderr, "type error: &&/|| requires booleans\n");
				type_delete(lt);
				type_delete(rt);
				return 0;
			}

			type_delete(lt);
			type_delete(rt);
			return type_create(TYPE_BOOLEAN, 0, 0);
		case EXPR_LIST:
		case EXPR_ARRAY_LIT:
			return 0;
		default:
			fprintf(stderr, "type error: unknown expression\n");
			return 0;
	}
}