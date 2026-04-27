#ifndef STACK_H
#define STACK_H

struct stack;

struct stack* stack_create();
void stack_delete(struct stack* s);

void stack_push(struct stack* s, void* item);
void* stack_pop(struct stack* s);

int stack_size(struct stack* s);

/* index 0 is the bottom, index (size-1) is the top */
void* stack_item(struct stack* s, int index);

#endif
