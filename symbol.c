#include <stdlib.h>
#include "symbol.h"

struct symbol* symbol_create(symbol_t kind, struct type* type, const char* name) {
	struct symbol* s = malloc(sizeof(*s));
	if (!s) return 0;

	s->kind = kind;
	s->type = type;
	s->name = (char*)name;
	s->which = 0;

	return s;
}

void symbol_delete(struct symbol* s) {
	if (!s) return;
	free(s);
}
