%{
#include "y.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int line_cnt = 1;
int char_cnt = 1;
int has_error = 0;
extern int error_cnt;
#define BUF_SIZE 4096
char line_buf[BUF_SIZE] = "";

void buf_append(const char* text){
    if(strlen(line_buf) + strlen(text) < BUF_SIZE - 1){
        strcat(line_buf, text);
    }else{
        printf("[Warning] Buffer full. some part of this line cannot be displayed correctly.\n");
    }
}

void print_line(){
    if(!has_error){
        const char* str = line_buf;
        int flag = 0;   // not all whitespace ?
        while(*str != '\0'){
            if(!isspace(*str)){
                flag = 1;
                break;
            }
            ++str;
        }
        if(flag){
            printf("Line %d: %s\n", line_cnt - 1, line_buf);
        }

    }
    line_buf[0] = '\0';
    char_cnt = 1;
    has_error = 0;
}

void pos_mantain(){
    yylloc.first_line = yylloc.last_line = line_cnt;
    yylloc.first_column = char_cnt;
    yylloc.last_column = char_cnt + yyleng - 1;
    char_cnt += yyleng;
}
%}
%option case-insensitive

%x COMMENT

Digit [0-9]
Int   (([1-9]{Digit}*)|0)
%%
"(*"            { pos_mantain();BEGIN(COMMENT);}
<COMMENT>"*)"   { pos_mantain();BEGIN(INITIAL);}
<COMMENT>\n     { ++line_cnt;char_cnt=1;print_line();}
<COMMENT>.|\r   { pos_mantain();}
<COMMENT><<EOF>> {
    pos_mantain();
    char_cnt = 1;
    fprintf(stderr, "[Lexical] ERROR - COMMENT TO EOF (Line %d)\n", line_cnt);
    BEGIN(INITIAL); 
}

"*)" {
    pos_mantain();
    has_error = 1;
    error_cnt++;
    fprintf(stderr, "Line %d, at char %d: [Lexical] invalid end of comment mark *). expected the start comment mark (* before it..\n", line_cnt, char_cnt - yyleng);
}

\n          { ++line_cnt;char_cnt=1;print_line();}
[ \t\r]+    { pos_mantain();buf_append(yytext);}




"program"    { pos_mantain();buf_append(yytext);return R_PROGRAM; }
"label"      { pos_mantain();buf_append(yytext);return R_LABEL; }
"const"      { pos_mantain();buf_append(yytext);return R_CONST; }
"type"       { pos_mantain();buf_append(yytext);return R_TYPE; }
"var"        { pos_mantain();buf_append(yytext);return R_VAR; }
"procedure"  { pos_mantain();buf_append(yytext);return R_PROCEDURE; }
"function"   { pos_mantain();buf_append(yytext);return R_FUNCTION; }
"begin"      { pos_mantain();buf_append(yytext);return R_BEGIN; }
"end"        { pos_mantain();buf_append(yytext);return R_END; }
"array"      { pos_mantain();buf_append(yytext);return R_ARRAY; }
"of"         { pos_mantain();buf_append(yytext);return R_OF; }
"record"     { pos_mantain();buf_append(yytext);return R_RECORD; }
"case"       { pos_mantain();buf_append(yytext);return R_CASE; }
"set"        { pos_mantain();buf_append(yytext);return R_SET; }
"file"       { pos_mantain();buf_append(yytext);return R_FILE; }
"goto"       { pos_mantain();buf_append(yytext);return R_GOTO; }
"if"         { pos_mantain();buf_append(yytext);return R_IF; }
"then"       { pos_mantain();buf_append(yytext);return R_THEN; }
"else"       { pos_mantain();buf_append(yytext);return R_ELSE; }
"while"      { pos_mantain();buf_append(yytext);return R_WHILE; }
"do"         { pos_mantain();buf_append(yytext);return R_DO; }
"repeat"     { pos_mantain();buf_append(yytext);return R_REPEAT; }
"until"      { pos_mantain();buf_append(yytext);return R_UNTIL; }
"for"        { pos_mantain();buf_append(yytext);return R_FOR; }
"to"         { pos_mantain();buf_append(yytext);return R_TO; }
"downto"     { pos_mantain();buf_append(yytext);return R_DOWNTO; }
"with"       { pos_mantain();buf_append(yytext);return R_WITH; }
"not"        { pos_mantain();buf_append(yytext);return R_NOT; }
"nil"        { pos_mantain();buf_append(yytext);return R_NIL; }
"in"         { pos_mantain();buf_append(yytext);return R_IN; }
"div"        { pos_mantain();buf_append(yytext);return R_DIV; }
"mod"        { pos_mantain();buf_append(yytext);return R_MOD; }
"and"        { pos_mantain();buf_append(yytext);return R_AND; }
"or"         { pos_mantain();buf_append(yytext);return R_OR; }
"integer"    { pos_mantain();buf_append(yytext);return R_INTEGER; }
"real"       { pos_mantain();buf_append(yytext);return R_REALTYPE; }
"string"     { pos_mantain();buf_append(yytext);return R_STRING; }



[a-zA-Z_][a-zA-Z0-9_]* {
    pos_mantain();
    buf_append(yytext);
    yylval.strVal = strdup(yytext); 
    if(yyleng > 15){
        fprintf(stderr, "Line %d, at char %d: Lexical Error - the id length %d exceed the size limit (length should be lower than 16).\n", line_cnt, char_cnt - yyleng, yyleng);
    }
    return ID; 
}

[0-9]+"."[0-9]+ {
    pos_mantain();
    buf_append(yytext);
    yylval.floatVal = atof(yytext); 
    return REAL; 
}

[0-9]+ {
    pos_mantain();
    buf_append(yytext);
    yylval.intVal = atoi(yytext); 
    return INT; 
}

\'[^\']*\'|\"[^\"]*\" { 
    pos_mantain();
    buf_append(yytext);
    yylval.strVal = strdup(yytext);
    int strsize = 0;
    for(int i=1;i<yyleng-1;++i){
        if(yytext[i]=='\'' && i+1<yyleng-1 && yytext[i+1]=='\''){
            ++i;
        }
        ++strsize;
    }

    if(strsize > 15){
        fprintf(stderr, "Line %d, at char %d: Lexical Error - The str length %d exceeds size limit (length should be lower than 32).\n", line_cnt, char_cnt - yyleng, strsize);
    }
    return STRING; 
}



":="                { pos_mantain();buf_append(yytext);return S_ASSIGN; }
"<>"                { pos_mantain();buf_append(yytext);return S_NE; }
"<="                { pos_mantain();buf_append(yytext);return S_LE; }
">="                { pos_mantain();buf_append(yytext);return S_GE; }
".."                { pos_mantain();buf_append(yytext);return S_DOTDOT; }
[=\-+*/<>:;,()\[\].] {
    pos_mantain();buf_append(yytext);
    return yytext[0]; 
}





<<EOF>> {
    if(strlen(line_buf) > 0 && !has_error){
        printf("Line %d: %s\n", line_cnt, line_buf);
    }
    return 0;
}

. {
    pos_mantain();buf_append(yytext);
    has_error = 1;
    error_cnt++;
    fprintf(stderr, "Line %d, at char %d: Lexical Error - Unrecognized character '%s'\n", line_cnt, char_cnt - yyleng, yytext);
}
%%