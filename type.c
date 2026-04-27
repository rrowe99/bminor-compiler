#include "type.h"
#include "param_list.h"

struct type * type_create( type_t kind, struct type *subtype, struct param_list *params ) {
	struct type *t = (struct type *)malloc(sizeof(*t));
	if (!t) {
		fprintf(stderr, "memory allocation failed: type_create()");
		exit(1);
	}

	t->kind = kind;
	t->subtype = subtype;
	t->params = params;
    t->size = -1; //negative 1 so that it will not be printed

    return t;
}

void type_print( struct type *t ) {
    if (!t) {
        fprintf(stderr, "\ntype printing failed\n");
        return;
    }

    switch (t->kind) {
        case TYPE_VOID: 
            printf("void");
            break;
	    case TYPE_BOOLEAN:
            printf("boolean");
            break;
	    case TYPE_CHARACTER:
            printf("char");
            break;
	    case TYPE_INTEGER:
            printf("integer");
            break;
	    case TYPE_STRING:
            printf("string");
            break;
	    case TYPE_ARRAY:
            printf("array [");
	        if (t->size >= 0) {
	            printf("%d", t->size);
	        }
	        printf("] ");
	        if(t->subtype) {
	            type_print(t->subtype);
	        }
            break;
	    case TYPE_FUNCTION:
            printf("function ");
            if (t->subtype) {
                type_print(t->subtype);
            }
            printf(" (");
            if (t->params) {
                param_list_print(t->params);
            }
            printf(")");
            break;
    }
}

int type_equals(struct type *a, struct type *b) {
	if (!a || !b) { return 0; }
	if (a->kind != b->kind) { return 0; }

	switch (a->kind) {
		case TYPE_ARRAY:
			return type_equals(a->subtype, b->subtype);
            break;
		case TYPE_FUNCTION:
			return (type_equals(a->subtype, b->subtype) && param_list_equal(a->params, b->params));
            break;
		case TYPE_BOOLEAN:
            return 1;
            break;
		case TYPE_CHARACTER:
            return 1; 
            break;
		case TYPE_INTEGER:
            return 1;
            break;
		case TYPE_STRING:
            return 1; 
            break;
		case TYPE_VOID:
			return 1;
		default:
			return 0;
	}
}

struct type * type_copy(struct type *t) {
	if (!t) { return 0; }

	struct type *temp = malloc(sizeof(*temp));
	if (!temp) { return 0; }

	temp->kind = t->kind;
    temp->size = t->size;
	temp->subtype = type_copy(t->subtype);
	temp->params = param_list_copy(t->params);

	return temp;
}

void type_delete(struct type *t) {
	if (!t) { return; }

	type_delete(t->subtype);
	if (t->params) {
		param_list_delete(t->params);
	}
	free(t);
}