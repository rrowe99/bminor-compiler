#include "decl.h"
#include "scope.h"
#include "param_list.h"
#include "type.h"
#include "codegen_state.h"

int local_count = 0;
int param_count = 0;


struct decl * decl_create( char *name, struct type *type, struct expr *value, struct stmt *code, struct decl *next )
{
	struct decl *d = (struct decl*)malloc(sizeof(*d));
	if (!d) {
		fprintf(stderr, "memory allocation failed: decl_create()");
		exit(1);
	}
	d->name = name;
	d->type = type;
	d->value = value;
	d->code = code;
	d->symbol = NULL;
	d->next = next;
	return d;
}

static void Indent_print(int level){
    for(int i = 0; i<level; i++){
                printf("    ");
            }
    return;
}

void decl_print( struct decl *d, int indent ) {
    if (!d) {
        fprintf(stderr, "\ndecl_print() failed\n");
        return;
    }

    Indent_print(indent);
    printf("%s: ", d->name);
	type_print(d->type);

	//functions, function body
	if (d->type->kind == TYPE_FUNCTION) {
		if (d->code) {
			printf(" = ");
			stmt_print(d->code, indent);
		} else {
			printf(";\n");
		}
	} else {             //variables
		if (d->value) {
			printf(" = ");
			expr_print(d->value);
		}
		printf(";\n");
	}

	if (d->next) {
		decl_print(d->next, indent);
	}
}

void decl_resolve( struct decl *d ) {
	if (!d) return;

	symbol_t kind = SYMBOL_LOCAL;
	if (scope_level() == 1) {
		kind = SYMBOL_GLOBAL;
	}

	struct symbol *existing = scope_lookup_current(d->name);

	if (existing) {

		if (existing->type && d->type &&
		    existing->type->kind == TYPE_FUNCTION &&
		    d->type->kind == TYPE_FUNCTION) {

			if (type_equals(existing->type, d->type)) {
				d->symbol = existing;
			} else {
				fprintf(stderr, "resolve error: function %s definition does not match prototype\n", d->name);
				d->symbol = existing;
			}

		} else {
			fprintf(stderr, "resolve error: %s already declared in this scope\n", d->name);
		}

	} else {
		d->symbol = symbol_create(kind, d->type, d->name);
		if (kind == SYMBOL_LOCAL) {
			d->symbol->which = local_count++;
		}

		scope_bind(d->name, d->symbol);
	}

	if (d->value) {
		expr_resolve(d->value);
	}

	if (d->type && d->type->kind == TYPE_FUNCTION && d->code) {
		scope_enter();
		local_count = 0;
		param_count = 0;


		if (d->type->params) {
			param_list_resolve(d->type->params);
		}

		stmt_resolve(d->code);
		if (d->symbol) d->symbol->which = local_count; // store #locals on function symbol
		scope_exit();
	}

	decl_resolve(d->next);
}

void decl_typecheck( struct decl *d ) {
	if (!d) return;

	if (d->type && d->type->kind != TYPE_FUNCTION) {
		if (d->value) {
			struct type *v = expr_typecheck(d->value);
			if (v) {
				if (!type_equals(d->type, v)) {
					fprintf(stderr, "type error: initializer for %s does not match declared type\n", d->name);
				}
				type_delete(v);
			}
		}
	}

	if (d->type && d->type->kind == TYPE_FUNCTION) {
		if (d->code) {
			stmt_typecheck(d->code, d->type->subtype);
		}
	}

	decl_typecheck(d->next);
}