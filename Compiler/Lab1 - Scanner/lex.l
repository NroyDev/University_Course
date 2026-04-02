%{
    #include<stdio.h>
    void cstr_tolower(char*);
    void print();
    int line_cnt = 1;
    int char_cnt = 1;
%}

%x COMMENT

space [ ]
nl \n|\r\n

/* Reserverd word */
%option caseless
Reserved (absolute|and|begin|break|case|const|continue|do|else|end|for|function|if|mod|nil|not|object|of|or|program|then|to|var|while|array|integer|double|write|writeln|string|float|read|array|integer|double|write|writeln|string|float)

/* ID */
Identifiers [_a-zA-Z][_0-9a-zA-Z]{0,14}
InvaildID [#^0-9][_0-9a-zA-Z]+|[_a-zA-Z][_0-9a-zA-Z]{15,}

Symbols [;:()><=[\]+\-*/.]|:=|==|<=|>= 

String '.{0,30}'
invalidString '.*'|'[^']*{Symbols}

/* For Real numbers */
InvalidFloat -?(\.[0-9]+|[0-9]+\.|0[0-9]+\.[0-9]+|[0-9]+\.[0-9]+0)
InvalidReal {InvalidFloat}

Int -?([0-9]+)
Float -?([0-9]+\.[0-9]+)
Exp -?(((([0-9]+\.[0-9]+)([eE][-+]?[0-9]+)?))|((([0-9]+)([eE][-+]?[0-9]+))))
Real {Int}|{Float}|{Exp}

%%
\(\* { BEGIN COMMENT; }
<COMMENT>\*\) { BEGIN 0; }
<COMMENT>[^\n] { ++char_cnt; }
<COMMENT>\n    { ++line_cnt;char_cnt=1; }

{Reserved} {
    printf("Line: %d, 1st char: %d, “%s” is a “reserved word”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{Identifiers} {
    printf("Line: %d, 1st char: %d, “%s” is a “ID”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{InvaildID} {
    printf("Line: %d, 1st char: %d, “%s” is an invalid “ID”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{Symbols} {
    printf("Line: %d, 1st char: %d, “%s” is a “symbol”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{InvalidReal} {
    printf("Line: %d, 1st char: %d, “%s” is an invalid “real”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{Real} {
    printf("Line: %d, 1st char: %d, “%s” is a “real”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{String} {
    printf("Line: %d, 1st char: %d, “%s” is a “string”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{invalidString} {
    printf("Line: %d, 1st char: %d, “%s” is an invalid “string”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}


{nl} {
    line_cnt += 1;
    char_cnt = 1;
}
{space} {
    char_cnt += 1;
}
.    { printf("invalid character %s\n", yytext); ++char_cnt; } 

%%
void cstr_tolower(char* str){
    while(str != NULL && *str != '\0'){
        if('A'<*str && *str<'Z'){
            *str |= (1<<5);
        }
    }

    return;
}
int main(){
    //printf("Start of program\n");
    yylex();
    //printf("End of program\n");
    //printf("\n\nThe number of characters: %d\n",charCount);
    //printf("The number of lines: %d\n", lineCount);
    return 0;
}
