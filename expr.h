#ifndef EXPR_H
#define EXPR_H

#include "symbol.h"
#include "type.h"
#include <stdlib.h>

typedef enum {
	EXPR_ADD,
	EXPR_SUB,
	EXPR_MUL,
	EXPR_DIV,
	EXPR_NAME,
	EXPR_MOD,
	EXPR_EXP,
	EXPR_EQ,
    EXPR_NE,
    EXPR_LT,
    EXPR_LE,
    EXPR_GT,
    EXPR_GE,
	EXPR_AND,
	EXPR_OR,
	EXPR_NOT,
	EXPR_ASSIGN,
	EXPR_CHAR_LITERAL,
    EXPR_INTEGER_LITERAL,
	EXPR_STRING_LITERAL,
	EXPR_FUNC_CALL,
	EXPR_ARR_ACC,
	EXPR_BOOL_LITERAL,
	EXPR_INC,
	EXPR_DEC,
	EXPR_LIST,
	EXPR_ARRAY_LIT
	/* many more kinds of exprs to add here */
} expr_t;

struct expr {
	/* used by all kinds of exprs */
	expr_t kind;
	struct expr *left;
	struct expr *right;

	/* used by various leaf exprs */
	const char *name;
	int literal_value;
	const char * string_literal;
	struct symbol *symbol;
};

struct expr * expr_create( expr_t kind, struct expr *left, struct expr *right );

struct expr * expr_create_name( const char *n );

struct expr * expr_create_integer_literal( int c );

struct expr * expr_create_boolean_literal( int c );

struct expr * expr_create_char_literal( char c );

struct expr * expr_create_string_literal( const char *str );

void expr_print( struct expr *e );

void expr_resolve( struct expr *e );

struct type * expr_typecheck( struct expr *e );

int expr_codegen(struct expr *e, FILE *out);

#endif
