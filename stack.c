#include <stdlib.h>
#include "stack.h"

struct stack_node {
	void* item;
	struct stack_node* next;
};

struct stack {
	struct stack_node* head;
	int size;
};

struct stack* stack_create() {
	struct stack* s = malloc(sizeof(*s));
	if (!s) { return 0; }
	s->head = 0;
	s->size = 0;
	return s;
}

void stack_delete(struct stack* s) {
	if (!s) { return; }

	while (s->head) {
		struct stack_node* n = s->head;
		s->head = n->next;
		free(n);
	}

	free(s);
}

void stack_push(struct stack* s, void* item) {
	if (!s) { return; }

	struct stack_node* n = malloc(sizeof(*n));
	if (!n) { return; }

	n->item = item;
	n->next = s->head;
	s->head = n;
	s->size++;
}

void* stack_pop(struct stack* s) {
	if (!s) { return 0; }
	if (!s->head) { return 0; }

	struct stack_node* n = s->head;
	void* item = n->item;

	s->head = n->next;
	free(n);
	s->size--;

	return item;
}

int stack_size(struct stack* s) {
	if (!s) { return 0; }
	return s->size;
}

// index 0 = bottom
// size - 1 = top
void* stack_item(struct stack* s, int index) {
	if (!s) { return 0; }
	if (index < 0) { return 0; }
	if (index >= s->size) { return 0; }

	//steps = indexes down from head (top)
	int steps = (s->size - 1) - index;

	struct stack_node* n = s->head;
	while (steps > 0 && n) {
		n = n->next;
		steps--;
	}

	if (!n) return 0;
	return n->item;
}

