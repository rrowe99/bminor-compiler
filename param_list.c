#include "param_list.h"
#include "scope.h"
#include "expr.h"
#include "codegen_state.h"

struct param_list * param_list_create( char *name, struct type *type, struct param_list *next ) {
	struct param_list *p = (struct param_list *)malloc(sizeof(*p));
	if (!p) {
		fprintf(stderr, "\nmemory allocation failed: param_list_create()\n");
		exit(1);
	}

	p->name = name;
	p->type = type;
	p->symbol = NULL;
	p->next = next;

	return p;
}

void param_list_print( struct param_list *a ) {
    if (!a) {
        fprintf(stderr, "\nparam_list_print() failed\n");
    }

    printf("%s: ", a->name);
    type_print(a->type);
    if (a->next) {
        printf(", ");
        param_list_print(a->next);
    }
}

void param_list_resolve( struct param_list *p ) {
	if (!p) { return; }

	if (scope_lookup_current(p->name)) {
		fprintf(stderr, "resolve error: parameter %s already declared\n", p->name);
	} else {
		p->symbol = symbol_create(SYMBOL_PARAM, p->type, p->name);
		p->symbol->which = param_count++;
		scope_bind(p->name, p->symbol);
	}

	if (p->next) {
		param_list_resolve(p->next);
	}
}

int param_list_equal( struct param_list *a, struct param_list *b ) {
	while (a && b) {
		if (!type_equals(a->type, b->type)) return 0;
		a = a->next;
		b = b->next;
	}
	if (a || b) return 0;
	return 1;
}

struct param_list * param_list_copy( struct param_list *p ) {
	if (!p) { return 0; }

	struct param_list *head = 0;
	struct param_list *tail = 0;

	while (p) {
		struct param_list *n = param_list_create(p->name, type_copy(p->type), 0);
		n->symbol = 0;

		if (!head) {
			head = n;
			tail = n;
		} else {
			tail->next = n;
			tail = n;
		}
		p = p->next;
	}

	return head;
}

void param_list_delete( struct param_list *p ) {
	while (p) {
		struct param_list *next = p->next;
		if (p->type) type_delete(p->type);
		free(p); //ast handles p->name
		p = next;
	}
}

int param_list_typecheck( struct param_list *p, struct expr *args ) {
	while (p && args) {
		if (args->kind != EXPR_LIST) {
			fprintf(stderr, "type error: wrong kind of expr for argument list\n");
			return 0;
		}

		struct type *at = expr_typecheck(args->left);
		if (!at) { return 0; }

		if (!type_equals(p->type, at)) {
			fprintf(stderr, "type error: argument type does not match parameter %s\n", p->name);
			type_delete(at);
			return 0;
		}

		type_delete(at);
		p = p->next;
		args = args->right;
	}

	if (p || args) {
		fprintf(stderr, "type error: too many / too few arguments\n");
		return 0;
	}

	return 1;
}