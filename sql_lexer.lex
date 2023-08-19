%{
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include "sql.h"
using namespace std;
char* remove_quote(char* str);
#include "sql_parser.h"
%}

delim [ \t\r]
newline \n
whitespace {delim}+
digit [0-9]
letter [A-Za-z_]
id {letter}({letter}|{digit})*
integer ([1-9]|[1-9]([0-9])+|0)
string \"(\\.|[^"\\])*\"

%%
{whitespace} {;}
{newline}       {return (NEWLINE);}
(create|CREATE) {return (CREATE);}
(select|SELECT) {return (SELECT);}
(from|FROM)     {return (FROM);}
(where|WHERE)   {return (WHERE);}
(and|AND)       {return (AND);}
(or|OR) {return (OR);}
(insert|INSERT) {return (INSERT);}
(into|INTO)     {return (INTO);}
(delete|DELETE) {return (DELETE);}
(update|UPDATE) {return (UPDATE);}
(set|SET)       {return (SET);}
(int|INT)       {return (TYPEINT);}
(char|CHAR)       {return (TYPECHAR);}
(use|USE)       {return (USE);}
(drop|DROP)       {return (DROP);}
(show|SHOW)     {return (SHOW);}
(tables|TABLES)     {return (TABLES);}
(databases|DATABASES)     {return (DATABASES);}
(table|TABLE)     {return (TABLE);}
(database|DATABASE)     {return (DATABASE);}
(values|VALUES)     {return (VALUES);}
(exit|EXIT)     {return (EXIT);}
{id}    {yylval.str = new string(yytext);return (ID);}
{string} {yylval.str = new string(remove_quote(yytext));return (STRING);}
{integer} {yylval.str = new string(yytext);return (NUM);}

"<"     {return (LT);}
"<="             {return (LE);}
">"             {return (GT);}
">="             {return (GE);}
"=="             {return (EQ);}
"="             {return {ASSIGN};}
","             {return (COMMA);}
"!="            {return (NEQ);}
";"             {return (SEM);}
"("             {return (LBR);}
")"             {return (RBR);}
"*"             {return (STAR);}
"."             {return (DOT);}
.       {;}
%%

int yywrap(void){
    return 1;
}

char* remove_quote(char* str){
    int len = strlen(str);
    if(!(str[0] == '\"' && str[len - 1] == '\"'))
        return str;
    str[len - 1] = '\0';
    return str + 1;
}
