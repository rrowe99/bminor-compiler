%{
#include <stdio.h>
#include <stdlib.h>

#include "decl.h"
#include "stmt.h"
#include "expr.h"
#include "type.h"
#include "param_list.h"

extern int yylex(void);
void yyerror(const char *s);

struct decl *parser_result = NULL; /* will be returned as AST node to tell if program succeeds */

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern int scan_error;

%}

%define api.value.type {void *} /* YYSTYPE is not assigned because I didn't use %union, must define */
%define parse.error verbose
%start program


/* returns tokens, numbered so that bison doesn't auto assign numbers */

%token TOKEN_ARRAY        1
%token TOKEN_NOT          2
%token TOKEN_BOOLEAN      3
%token TOKEN_CHAR         4
%token TOKEN_ELSE         5
%token TOKEN_FALSE        6
%token TOKEN_FOR          7
%token TOKEN_FUNCTION     8
%token TOKEN_IF           9
%token TOKEN_INTEGER     10
%token TOKEN_PRINT       11
%token TOKEN_RETURN      12
%token TOKEN_STRING      13
%token TOKEN_TRUE        14
%token TOKEN_VOID        15
%token TOKEN_WHILE       16

%token TOKEN_CHAR_LIT    17
%token TOKEN_STRING_LIT  18
%token TOKEN_PLUS        19
%token TOKEN_MINUS       20
%token TOKEN_INCREMENT   22
%token TOKEN_DECREMENT   23
%token TOKEN_IDENTIFIER  24
%token TOKEN_EXP         25
%token TOKEN_MUL         26
%token TOKEN_DIV         27
%token TOKEN_MOD         28
%token TOKEN_LT          29
%token TOKEN_LE          30
%token TOKEN_GT          31
%token TOKEN_GE          32
%token TOKEN_EQ          33
%token TOKEN_NE          34
%token TOKEN_AND         35
%token TOKEN_OR          36
%token TOKEN_ASSIGN      37
%token TOKEN_INT_LIT     38
%token TOKEN_COLON       39
%token TOKEN_SEMI        40
%token TOKEN_COMMA       41
%token TOKEN_LPAREN      42
%token TOKEN_RPAREN      43
%token TOKEN_LBRACKET    44
%token TOKEN_RBRACKET    45
%token TOKEN_LBRACE      46
%token TOKEN_RBRACE      47

%token TOKEN_INVALID     99

/*
%type <decl> decl decl_list program var_decl array_decl func_proto func_def
%type <stmt> stmt matched_stmt unmatched_stmt other_stmt for_stmt block block_items block_item
%type <expr> expr assign_expr lval equality_expr relational_expr additive_expr multiplicative_expr power_expr postfix_expr primary expr_list arg_list opt_expr opt_init opt_array_init opt_print_list print_list args_opt
%type <type> type_no_void type_or_void param_type
%type <param_list> param param_list params_opt
*/

%%

/*high level eque stuff here*/
program
    : decl_list 
      { parser_result = $1; } /* if parser succeeds, parser_result returns the AST node created */
    ;

decl_list
    : %empty
      { $$ = NULL; }
    | decl decl_list 
    {
      $$ = $1;
      struct decl *tail = (struct decl *) $1;
        while (tail && tail->next) {
        tail = tail->next;
      }
      if (tail) {
        tail->next = (struct decl *) $2;
      } else {
        $$ = $2;
      }
    }
    ;

decl
    : var_decl
      { $$ = $1; }
    | func_proto
      { $$ = $1; }
    | func_def
      { $$ = $1; }
    ;

type_no_void
    : TOKEN_INTEGER
      { $$ = type_create(TYPE_INTEGER, NULL, NULL); }
    | TOKEN_BOOLEAN
      { $$ = type_create(TYPE_BOOLEAN, NULL, NULL); }
    | TOKEN_CHAR 
      { $$ = type_create(TYPE_CHARACTER, NULL, NULL); }
    | TOKEN_STRING
      { $$ = type_create(TYPE_STRING, NULL, NULL); }
    | TOKEN_ARRAY TOKEN_LBRACKET TOKEN_RBRACKET type_no_void  /* nested array */
      {
        struct type *arr = type_create(TYPE_ARRAY, $4, NULL);
        arr->size = -1;   /* -1 so it won't be printed */
        $$ = arr;
      }
    | TOKEN_ARRAY TOKEN_LBRACKET TOKEN_INT_LIT TOKEN_RBRACKET type_no_void /* nested array with number */
      {
        struct type *arr = type_create(TYPE_ARRAY, $5, NULL);
        arr->size = atoi((char *)$3);
        $$ = arr;
      }
    ;

type_or_void
    : type_no_void
      { $$ = $1; }
    | TOKEN_VOID
      { $$ = type_create(TYPE_VOID, NULL, NULL); }
    ;

var_decl
    : TOKEN_IDENTIFIER TOKEN_COLON type_no_void opt_init TOKEN_SEMI
      { $$ = decl_create((char *)$1, $3, $4, NULL, NULL); }
    ;

opt_init
    : %empty
      { $$ = NULL; }
    | TOKEN_ASSIGN expr
      { $$ = $2; }
    ;

opt_array_init
    : %empty
      { $$ = NULL; }
    | TOKEN_ASSIGN TOKEN_LBRACE expr_list TOKEN_RBRACE
      { $$ = expr_create(EXPR_ARRAY_LIT, $3, NULL); }
    ;

opt_expr_list
    : %empty
      { $$ = NULL; }
    | expr_list
      { $$ = $1; }
    ;

expr_list
    : expr
      { $$ = expr_create(EXPR_LIST, $1, NULL); }
    | expr TOKEN_COMMA expr_list
      { $$ = expr_create(EXPR_LIST, $1, $3); }
    ;

func_proto
    : TOKEN_IDENTIFIER TOKEN_COLON TOKEN_FUNCTION type_or_void TOKEN_LPAREN params_opt TOKEN_RPAREN TOKEN_SEMI
      {
        struct type *f = type_create(TYPE_FUNCTION, $4, $6);
        $$ = decl_create((char *)$1, f, NULL, NULL, NULL); 
      }
    ;

func_def
    : TOKEN_IDENTIFIER TOKEN_COLON TOKEN_FUNCTION type_or_void TOKEN_LPAREN params_opt TOKEN_RPAREN TOKEN_ASSIGN block
      {
        struct type *f = type_create(TYPE_FUNCTION, $4, $6);
        $$ = decl_create((char *)$1, f, NULL, $9, NULL);
      }
    ;

params_opt
    : %empty
      { $$ = NULL; }
    | param_list
      { $$ = $1; }
    ;

param_list
    : param
      { $$ = $1; }
    | param TOKEN_COMMA param_list
      {
        $$ = $1;
        struct param_list *tail = (struct param_list *) $1;
        while(tail->next) {
          tail = tail->next;
        }
      tail->next = (struct param_list *) $3;
      }
    ;

param
    : TOKEN_IDENTIFIER TOKEN_COLON param_type 
      { $$ = param_list_create((char *)$1, $3, NULL); }
    ;

param_type
    : type_no_void 
      { $$ = $1; }
    ;

block
    : TOKEN_LBRACE block_items TOKEN_RBRACE
      { $$ = stmt_create(STMT_BLOCK, NULL, NULL, NULL, NULL, $2, NULL, NULL); }
    ;

block_items
    : %empty
      { $$ = NULL; }
    | block_item block_items
      {
        if ($2 == NULL) {
          $$ = $1;
        } else {
          struct stmt *tail = (struct stmt *) $1;
          while (tail->next) {
            tail = tail->next;
          }
          tail->next = (struct stmt *) $2;
          $$ = $1;
        }
      }
    ;

block_item
    : var_decl
      { $$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL, NULL); }
    | stmt
      { $$ = $1; }
    ;

stmt
    : matched_stmt
      { $$ = $1; }
    | unmatched_stmt
      { $$ = $1; }
    ;

matched_stmt
    : other_stmt
      { $$ = $1; }
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN matched_stmt TOKEN_ELSE matched_stmt
      { $$ = stmt_create(STMT_IF_ELSE, NULL, $3, NULL, NULL, $5, $7, NULL); }
    ;

unmatched_stmt
    : TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt
      { $$ = stmt_create(STMT_IF_ELSE, NULL, $3, NULL, NULL, $5, NULL, NULL); }
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN matched_stmt TOKEN_ELSE unmatched_stmt
      { $$ = stmt_create(STMT_IF_ELSE, NULL, $3, NULL, NULL, $5, $7, NULL); }
    ;

other_stmt
    : expr TOKEN_SEMI
      { $$ = stmt_create(STMT_EXPR, NULL, NULL, $1, NULL, NULL, NULL, NULL); }
    | TOKEN_RETURN opt_expr TOKEN_SEMI
      { $$ = stmt_create(STMT_RETURN, NULL, NULL, $2, NULL, NULL, NULL, NULL); }
    | TOKEN_PRINT opt_print_list TOKEN_SEMI
      { $$ = stmt_create(STMT_PRINT, NULL, NULL, $2, NULL, NULL, NULL, NULL); } 
        /* ends up creating a list using the expr_list rules */
    | for_stmt
      { $$ = $1; }
    | block
      { $$ = $1; }
    | TOKEN_WHILE TOKEN_LPAREN expr TOKEN_RPAREN stmt
      { $$ = stmt_create(STMT_WHILE, NULL, NULL, $3, NULL, $5, NULL, NULL); }
    ;

opt_expr
    : %empty
      { $$ = NULL; }
    | expr
      { $$ = $1; }
    ;

opt_print_list
    : %empty
      { $$ = NULL; }
    | print_list
      { $$ = $1; }
    ;

print_list
    : expr
      { $$ = expr_create(EXPR_LIST, $1, NULL); }
    | expr TOKEN_COMMA print_list
      { $$ = expr_create(EXPR_LIST, $1, $3); }
    ;

for_stmt
    : TOKEN_FOR TOKEN_LPAREN opt_expr TOKEN_SEMI opt_expr TOKEN_SEMI opt_expr TOKEN_RPAREN stmt
      { $$ = stmt_create(STMT_FOR, NULL, $3, $5, $7, $9, NULL, NULL); }
      /*      
         $3 = loop variable init
         $5 = condition
         $7 = thing you do after a loop
         $9 = loop body
      */
    ;

expr
    : assign_expr
      { $$ = $1; }
    ;

assign_expr
    : lval TOKEN_ASSIGN assign_expr
      { $$ = expr_create(EXPR_ASSIGN, $1, $3); }
    | logical_or_expr
      { $$ = $1; }
    ;

logical_or_expr
    : logical_and_expr
      { $$ = $1; }
    | logical_or_expr TOKEN_OR logical_and_expr
      { $$ = expr_create(EXPR_OR, $1, $3); }
    ;

logical_and_expr
    : equality_expr
      { $$ = $1; }
    | logical_and_expr TOKEN_AND equality_expr
      { $$ = expr_create(EXPR_AND, $1, $3); }
    ;

lval
    : postfix_expr
      { $$ = $1; }
    ;

equality_expr
    : relational_expr
      { $$ = $1; }
    | equality_expr TOKEN_EQ relational_expr
      { $$ = expr_create(EXPR_EQ, $1, $3); }
    | equality_expr TOKEN_NE relational_expr
      { $$ = expr_create(EXPR_NE, $1, $3); }
    ;

relational_expr
    : additive_expr
      { $$ = $1; }
    | relational_expr TOKEN_LT additive_expr
      { $$ = expr_create(EXPR_LT, $1, $3); }
    | relational_expr TOKEN_LE additive_expr
      { $$ = expr_create(EXPR_LE, $1, $3); }
    | relational_expr TOKEN_GT additive_expr
    { $$ = expr_create(EXPR_GT, $1, $3); }
    | relational_expr TOKEN_GE additive_expr
      { $$ = expr_create(EXPR_GE, $1, $3); }
    ;

additive_expr
    : multiplicative_expr
      { $$ = $1; }
    | additive_expr TOKEN_PLUS  multiplicative_expr
      { $$ = expr_create(EXPR_ADD, $1, $3); }
    | additive_expr TOKEN_MINUS multiplicative_expr
      { $$ = expr_create(EXPR_SUB, $1, $3); }
    ;

multiplicative_expr
    : power_expr
      { $$ = $1; }
    | multiplicative_expr TOKEN_MUL power_expr
      { $$ = expr_create(EXPR_MUL, $1, $3); }
    | multiplicative_expr TOKEN_DIV power_expr
      { $$ = expr_create(EXPR_DIV, $1, $3); }
    | multiplicative_expr TOKEN_MOD power_expr
      { $$ = expr_create(EXPR_MOD, $1, $3); }
    ;

power_expr
    : postfix_expr
      { $$ = $1; }
    | postfix_expr TOKEN_EXP power_expr
      { $$ = expr_create(EXPR_EXP, $1, $3); }
    ;

postfix_expr
    : primary
      { $$ = $1; }
    | postfix_expr TOKEN_LPAREN args_opt TOKEN_RPAREN
      {
        $$ = expr_create(EXPR_FUNC_CALL, $3, NULL);
        struct expr *call = (struct expr *) $$;
        struct expr *fn   = (struct expr *) $1;
        call->name = fn->name;
      }
    | postfix_expr TOKEN_LBRACKET expr TOKEN_RBRACKET
      { $$ = expr_create(EXPR_ARR_ACC, $1, $3); }
    | postfix_expr TOKEN_INCREMENT
      { $$ = expr_create(EXPR_INC, $1, NULL); }
    | postfix_expr TOKEN_DECREMENT
      { $$ = expr_create(EXPR_DEC, $1, NULL); }
    ;

args_opt
    : %empty
      { $$ = NULL; }
    | arg_list
      { $$ = $1; }
    ;

arg_list
    : expr
      { 
        $$ = expr_create(EXPR_LIST, $1, NULL);
      }
    | expr TOKEN_COMMA arg_list
      { 
        $$ = expr_create(EXPR_LIST, $1, $3);
      }
    ;

primary
    : TOKEN_NOT primary
      { $$ = expr_create(EXPR_NOT, $2, NULL); }
    | TOKEN_IDENTIFIER
      { $$ = expr_create_name((char *)$1); }
    | TOKEN_INT_LIT
      { $$ = expr_create_integer_literal(atoi((char *)$1)); }
    | TOKEN_CHAR_LIT
      {
        struct expr *e = expr_create(EXPR_CHAR_LITERAL, NULL, NULL);
        e->string_literal = (char *)$1;   /* literally stored as '\n' */
        $$ = e;
      }
    | TOKEN_STRING_LIT
      {
        struct expr *e = expr_create(EXPR_STRING_LITERAL, NULL, NULL);
        e->string_literal = (char *)$1;   /* raw text like "hi\n" */
        $$ = e;
      }
    | TOKEN_TRUE
      { $$ = expr_create_boolean_literal(1); }
    | TOKEN_FALSE
      { $$ = expr_create_boolean_literal(0); }
    | TOKEN_LPAREN expr TOKEN_RPAREN
      { $$ = $2; }  /* it uses the expr rules */
    | TOKEN_LBRACE opt_expr_list TOKEN_RBRACE
    { $$ = expr_create(EXPR_ARRAY_LIT, $2, NULL); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "parse error: %s at line %d\n", s, yylineno);
}
