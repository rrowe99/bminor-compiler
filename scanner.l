%option noyywrap
%option noinput
%option nounput
%option yylineno
%option never-interactive

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

int scan_error = 0;
extern void *yylval;
%}

/* handles comments, character literals, string literals */
%x COMMENT STRING 

/* macros */
ID      [A-Za-z_][A-Za-z0-9_]*
INT     [0-9]+
ESC_SEQ \\[nt\\'"?]

%%

[ \t\r\f\v]+        { } /* whitespace */
\n                  { } 
\/\/[^\n]*          { }

"/*"                { BEGIN(COMMENT); } /* block comments */
<COMMENT>"*/"       { BEGIN(INITIAL); }      
<COMMENT>\n         { }          
<COMMENT>[^*\n]+    { }                       
<COMMENT>\*+        { }

array               { return TOKEN_ARRAY; } 
boolean             { return TOKEN_BOOLEAN; }
char                { return TOKEN_CHAR; }
else                { return TOKEN_ELSE; }
false               { return TOKEN_FALSE; }
for                 { return TOKEN_FOR; }
function            { return TOKEN_FUNCTION; }
if                  { return TOKEN_IF; }
integer             { return TOKEN_INTEGER; }
print               { return TOKEN_PRINT; }
return              { return TOKEN_RETURN; }
string              { return TOKEN_STRING; }
true                { return TOKEN_TRUE; }
void                { return TOKEN_VOID; }
while               { return TOKEN_WHILE; }

{ID}                { yylval = strdup(yytext); return TOKEN_IDENTIFIER; }

\+\+                { return TOKEN_INCREMENT; }
\-\-                { return TOKEN_DECREMENT; }
\*\*                { return TOKEN_EXP; }
\^                  { return TOKEN_EXP; }
\<\=                { return TOKEN_LE; }
\>\=                { return TOKEN_GE; }
\=\=                { return TOKEN_EQ; }
\!\=                { return TOKEN_NE; }
\!                  { return TOKEN_NOT; }
\+                  { return TOKEN_PLUS; }
\-                  { return TOKEN_MINUS; }
\*                  { return TOKEN_MUL; }
\/                  { return TOKEN_DIV; }
\%                  { return TOKEN_MOD; }
\<                  { return TOKEN_LT; }
\>                  { return TOKEN_GT; }
\=                  { return TOKEN_ASSIGN; }
\&\&                { return TOKEN_AND; }
\|\|                { return TOKEN_OR; }

:                   { return TOKEN_COLON; }
;                   { return TOKEN_SEMI; }
,                   { return TOKEN_COMMA; }
\(                  { return TOKEN_LPAREN; }
\)                  { return TOKEN_RPAREN; }
\[                  { return TOKEN_LBRACKET; }
\]                  { return TOKEN_RBRACKET; }
\{                  { return TOKEN_LBRACE; }
\}                  { return TOKEN_RBRACE; }

{INT}               { yylval = strdup(yytext); return TOKEN_INT_LIT; }

\"([^"\\\n]|\\.)*\" { yylval = strdup(yytext); return TOKEN_STRING_LIT; }

\'([^'\\\n]|\\.)\'  { yylval = strdup(yytext); return TOKEN_CHAR_LIT; }

. {
    scan_error = 1;
    fprintf(stderr, "scan error: unrecognized character '%c' at line %d\n", yytext[0], yylineno);
    return TOKEN_INVALID;
}

<<EOF>> {
    return 0;
}

%%
/* no yywrap() because %option noyywrap */