#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"
#include "stack.h"
#include "symbol.h"

static struct stack* s1 = 0;

void scope_enter() {
	if (!s1) s1 = stack_create();

	struct hash_table* h = hash_table_create(0, 0);
	stack_push(s1, h);
}

void scope_exit() {
	struct hash_table* h = stack_pop(s1);
	if (h) hash_table_delete(h);
}

int scope_level() {
	return stack_size(s1);
}

void scope_bind(const char* name, struct symbol* sym) {
	int top = stack_size(s1) - 1;
	struct hash_table* h = stack_item(s1, top);
	hash_table_insert(h, name, sym);
}

struct symbol* scope_lookup(const char* name) {
	int i;
	for (i = stack_size(s1) - 1; i >= 0; i--) {
		struct hash_table* h = stack_item(s1, i);
		struct symbol* s = hash_table_lookup(h, name);
		if (s) return s;
	}
	return 0;
}

struct symbol* scope_lookup_current(const char* name) {
	int top = stack_size(s1) - 1;
	struct hash_table* h = stack_item(s1, top);
	return hash_table_lookup(h, name);
}
