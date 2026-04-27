
#ifndef PARAM_LIST_H
#define PARAM_LIST_H


#include <stdio.h>
#include <stdlib.h>
#include "symbol.h"
#include "type.h"

struct expr;

struct param_list {
	char *name;
	struct type *type;
	struct symbol *symbol;
	struct param_list *next;
};

struct param_list * param_list_create( char *name, struct type *type, struct param_list *next );

void param_list_print( struct param_list *a );

void param_list_resolve( struct param_list *p );

int param_list_equal( struct param_list *a, struct param_list *b );

struct param_list * param_list_copy( struct param_list *p );

void param_list_delete( struct param_list *p );

int param_list_typecheck( struct param_list *p, struct expr *args );

#endif
