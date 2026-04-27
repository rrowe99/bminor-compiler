#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "param_list.h"
#include "type.h"
#include "scope.h"


int yylex(void);
int yyparse(void);
void yyrestart(FILE*);

extern FILE *yyin;
extern int scan_error;

extern struct decl *parser_result; //AST root node

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <sourcefile.bminor>\n", prog);
}

int main(int argc, char **argv)
{
    if(argc != 2)
    { usage(argv[0]); return 1; }

    yyin = fopen(argv[1], "r");

    if (!yyin) { 
        perror("fopen"); return 1; 
    }
    yyrestart(yyin);

    scan_error = 0; //the scanner file will set scan_error to 1 if an error occurs.
    int rc = yyparse(); //returns 0 if parse succeeds

    fclose(yyin);

    if (scan_error) {
        fprintf(stderr, "\nscan failed\n");
        return 1;
    }

    if (rc != 0) {
        fprintf(stderr, "\nparse failed\n");
        return 1;
    }

    if (parser_result) {

    //name resolution
    scope_enter();               
    decl_resolve(parser_result);
    scope_exit();

    decl_typecheck(parser_result);

    decl_codegen(parser_result, stdout);


    /* for now, keep printing so you can still see something */
    /* decl_print(parser_result, 0); */
    }

    printf("\n");

    return 0;
}