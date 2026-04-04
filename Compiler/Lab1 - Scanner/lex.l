%{
    #include<stdio.h>
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
Int -?([0-9]+)
InvaildID [#^0-9][_0-9a-zA-Z]+|[#^]|[_a-zA-Z][_0-9a-zA-Z]{15,}

Symbols [;:()><=[\]+\-*/.,]|:=|==|<=|>= 

String '.{0,30}'
/* invalidString '.*' */
invalidString '.*'|'[^'\n]*{Symbols}

/* For Real numbers */
InvalidFloat -?(\.[0-9]+|[0-9]+\.|0[0-9]+\.[0-9]+|[0-9]+\.[0-9]+0)
InvalidExp -?([0-9]+\.[0-9]+)([eE][-+]?)
InvalidReal {InvalidFloat}|{InvalidExp}

Float -?([0-9]+\.[0-9]+)
Exp -?(((([0-9]+\.[0-9]+)([eE][-+]?[0-9]+)?))|((([0-9]+)([eE][-+]?[0-9]+))))
Real {Int}|{Float}|{Exp}

%%
\*\) {
	// Invalid Comments
	printf("Line: %d, 1st char: %d, “%s” is an invalid “comment”, missing (* before *).\n", line_cnt, char_cnt, yytext);
	char_cnt += yyleng;
}
\(\* { 
	// Comment start
	printf("Line: %d, 1st char: %d, “%s", line_cnt, char_cnt, yytext); 
	char_cnt += yyleng;
	BEGIN COMMENT;
}
<COMMENT>\*\) { 
	// Comment end
	printf("%s” is a “comment”.\n", yytext);
	char_cnt += yyleng; 
	BEGIN 0;
}
<COMMENT>. { 
        printf("%s", yytext);
        char_cnt += yyleng; 
}
<COMMENT>\n    { 
	printf("” is a “comment”.\n");
	++line_cnt;char_cnt=1; 
}
<COMMENT><<EOF>> {
	printf("” is an invalid “comment”, the quote reaches the end of file.\n");
}

{nl} {
    line_cnt += 1;
    char_cnt = 1;
}
{space} {
    char_cnt += 1;
}


{Reserved} {
    printf("Line: %d, 1st char: %d, “%s” is a “reserved word”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{Identifiers} {
    printf("Line: %d, 1st char: %d, “%s” is a “ID”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{Int} {
    /* Int must be declared before Invaild ID, for the case of 123*/
    printf("Line %d, 1st char: %d, “%s” is a “integer”.\n", line_cnt, char_cnt, yytext);
    char_cnt += yyleng;
}
{InvaildID} {
    printf("Line: %d, 1st char: %d, “%s” is an invalid “ID”.", line_cnt, char_cnt, yytext);
    if(yyleng > 15){
        printf(" the id length %d exceed the size limit (length should be lower than 16).", yyleng);
    }
    if(yyleng>0 && !( ('a'<=yytext[0] && yytext[0]<='z') || ('A'<=yytext[0] && yytext[0]<='Z') || yytext[0]=='_') ){
        printf(" The begin of ID SHOULD be only “Alpahbet” or “_”, “%c” is an invalid.", yytext[0]);
    }
    printf("\n");
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
    printf("Line: %d, 1st char: %d, “%s” is a “string”. Interpret as “", line_cnt, char_cnt, yytext);
    for(int i=1;i<yyleng-1;++i){
        printf("%c", yytext[i]);
        if(yytext[i]=='\'' && i+1<yyleng-1 && yytext[i+1]=='\''){
            ++i;
        }
    }
    printf("”\n");
    char_cnt += yyleng;
}
{invalidString} {
    printf("Line: %d, 1st char: %d, “%s” is an invalid “string”.", line_cnt, char_cnt, yytext);
    if(yyleng-2>30){
        printf(" The str length %d exceeds size limit (length should be lower than 31)\n", yyleng);
    }else{
        printf(" Is it missing ' at the end of string?\n"); 
    }
    char_cnt += yyleng;
}
.    { printf("Line:%d, 1st char: %d, “%s” is an invalid symbol.\n", line_cnt, char_cnt,  yytext); ++char_cnt; } 

%%
int main(){
    yylex();
    return 0;
}
