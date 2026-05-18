%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int line_cnt;
extern int char_cnt;
extern int yyleng;
extern int has_error;
extern char* line_buf;
int error_cnt = 0;  // total error count
int yylex(void);
void yyerror(const char *s);

// ----------------------- Symbol Table -----------------------
// 資料型態
typedef enum { TYPE_INT, TYPE_REAL, TYPE_STRING, TYPE_UNKNOWN } DataType;
struct Symbol {
    char name[64];
    DataType type;
};

#define sym_table_size 1024
struct Symbol sym_table[sym_table_size];
int sym_count = 0;
char temp_ids[20][50];  // 暫存宣告時的變數名稱 (eg. i, j : integer)
int temp_id_count = 0;
void add_symbol(char* name, DataType type);
DataType get_type(char* name, int line, int col);
void get_type_name(DataType dt, char* buf, const int buf_size);
%}
%define parse.error verbose    
%union {
    int intVal;
    float floatVal;
    char* strVal;
    int data_type; 
}

// ----------------------- Tokens -----------------------
%token R_PROGRAM    "program"
%token R_VAR        "var"
%token R_BEGIN      "begin"
%token R_END        "end"
%token R_INTEGER    "integer"
%token R_REALTYPE   "real"
%token R_STRING     "string"
%token R_ARRAY      "array"
%token R_OF         "of"
%token R_IF         "if"
%token R_THEN       "then"
%token R_ELSE       "else"
%token R_FOR        "for"
%token R_DO         "do"
%token R_TO         "to"
%token R_DOWNTO     "downto"
%token R_DIV        "div"
%token R_MOD        "mod"
%token R_LABEL      "label"
%token R_CONST      "const"
%token R_TYPE       "type"
%token R_PROCEDURE  "procedure"
%token R_FUNCTION   "function"
%token R_RECORD     "record"
%token R_CASE       "case"
%token R_SET        "set"
%token R_FILE       "file"
%token R_GOTO       "goto"
%token R_WHILE      "while"
%token R_REPEAT     "repeat"
%token R_UNTIL      "until"
%token R_WITH       "with"
%token R_NOT        "not"
%token R_NIL        "nil"
%token R_IN         "in"
%token R_AND        "and"
%token R_OR         "or"

%token S_ASSIGN     ":="
%token S_DOTDOT     ".."
%token S_GE         ">="
%token S_LE         "<="
%token S_NE         "<>"

%token <strVal> ID      "identifier"
%token <strVal> STRING  "string literal"
%token <intVal> INT     "integer literal"
%token <floatVal> REAL  "real literal"

%type <data_type> type standtype arraytype exp simpexp term factor varid
%left '+' '-'
%left '*' '/' R_DIV R_MOD
%precedence UMINUS UPLUS

%nonassoc R_THEN
%nonassoc R_ELSE
%%

prog        : R_PROGRAM prog_name ';' R_VAR dec_list ';' R_BEGIN stmt_list R_END '.'
            | error R_BEGIN stmt_list R_END '.'
            ;

prog_name   : ID            { free($1); }
            ;

dec_list    : dec
            | dec_list ';' dec
            ; 

dec         : id_list ':' type {    /*把下面declare 的東東存到 Symbol Table */
                for(int i = 0; i < temp_id_count; i++){
                    add_symbol(temp_ids[i], $3);
                }
                temp_id_count = 0; 
            }
            | id_list S_ASSIGN type {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: ':' excepted, but ':=' found.\n", @2.first_line, @2.first_column);
                for(int i = 0; i < temp_id_count; i++){     /* 我是覺得這邊宣告要全部但雕啦 但作業給的範例輸出那樣寫... */
                    add_symbol(temp_ids[i], $3);
                }
            }
            | error         { temp_id_count = 0; /* 出錯=>這行宣告的全但雕 */}
            ;

type        : standtype     { $$ = $1; }
            | arraytype     { $$ = $1; } 
            ;

standtype   : R_INTEGER     { $$ = TYPE_INT; }
            | R_REALTYPE    { $$ = TYPE_REAL; }
            | R_STRING      { $$ = TYPE_STRING; }
            ;

arraytype   : R_ARRAY '[' INT S_DOTDOT INT ']' R_OF standtype       { $$ = $8;}
            ;

id_list     : ID                { strcpy(temp_ids[temp_id_count++], $1);free($1);}
            | id_list ',' ID    { strcpy(temp_ids[temp_id_count++], $3); free($3); }
            ;

stmt_list   : stmt
            | stmt_list ';' stmt
            | stmt_list ';'
            ;

stmt        : assign
            | call_func
            | for_stmt
            | ifstmt
            | ID                { free($1); }
            | error             /* 讓下面報調的時候 自己往上看到error這個token 讓這個 stmt match error token，再往上到 stmt_list 看到';'接續正常得幾個token 恢復狀態 */
            ;

assign      : varid S_ASSIGN exp {
                if($1 != $3 && $1 != TYPE_UNKNOWN && $3 != TYPE_UNKNOWN){
                    has_error = 1;
                    ++error_cnt;
                    const int temp_buf_size = 32;
                    char buf1[temp_buf_size];
                    char buf2[temp_buf_size];
                    get_type_name($1, buf1, temp_buf_size);
                    get_type_name($3, buf2, temp_buf_size);
                    fprintf(stderr, "Line %d, at char %d: Type mismatch (expected %s but %s found) in assignment.\n", @2.first_line, @2.first_column, buf1, buf2);
                }
            }
            | varid '=' exp {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: excepted ':=', but '=' found. Notice that = in Pascal is to check equal. Singal expression in Pascal is invalid.\n", @2.first_line, @2.first_column);
            }
            ;

ifstmt      : R_IF exp R_THEN body
            | R_IF exp R_THEN body R_ELSE body
            /* Error Area */
            | R_IF exp ')' R_THEN body {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: unmatched (), expected '(' between if and expression\n", @2.first_line, @2.first_column);
            }
            | R_IF '(' exp R_THEN body {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: unmatched (), expected ')' between expression and 'then'\n", @4.first_line, @4.first_column);
            }
            | R_IF exp body {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: expected 'then' after the expression before the body part\n", @3.first_line, @3.first_column);
            }
            | R_IF exp ')' body {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: unmatched (), expected '(' between if and expression\n", @2.first_line, @2.first_column);
                fprintf(stderr, "Line %d, at char %d: expected 'then' after the expression before the body part\n", @4.first_line, @4.first_column);
            }
            | R_IF '(' exp body {
                has_error = 1;
                ++error_cnt;
                fprintf(stderr, "Line %d, at char %d: unmatched (), expected ')' after expression\n", @4.first_line, @4.first_column);
                fprintf(stderr, "Line %d, at char %d: expected 'then' after the expression before the body part\n", @4.first_line, @4.first_column);
            }
            /* else 漏了沒有關西 因為那個應該算是 logic error，根本沒辦法在這裡判斷後面的 statement 是 else 的part，後面的statement很可能是一般的statement*/
            ;

exp         : simpexp { $$ = $1; }
            | exp relop simpexp { $$ = TYPE_INT; }
            ;

relop       : '<' | '>' | '=' | S_GE | S_LE | S_NE 
            ;

simpexp     : term { $$ = $1; }
            | simpexp '+' term {
                if($1 != $3 && $1 != TYPE_UNKNOWN && $3 != TYPE_UNKNOWN){
                    has_error=1;
                    ++error_cnt;
                    const int temp_buf_size = 32;
                    char buf1[temp_buf_size];
                    char buf2[temp_buf_size];
                    get_type_name($1, buf1, temp_buf_size);
                    get_type_name($3, buf2, temp_buf_size);
                    fprintf(stderr, "Line %d, at char %d: Type mismatch (left is %s but right is %s), cannot add different types.\n", @2.first_line, @2.first_column, buf1, buf2);
                    $$ = TYPE_UNKNOWN;
                }else if($1 == TYPE_STRING){
                    has_error=1;
                    ++error_cnt;
                    fprintf(stderr, "Line %d, at char %d: Cannot perform arithmetic on strings.\n", @2.first_line, @2.first_column);
                    $$ = TYPE_UNKNOWN;
                }else{
                    $$ = $1; 
                }
            }
            | simpexp '-' term {
                if($1 != $3 && $1 != TYPE_UNKNOWN && $3 != TYPE_UNKNOWN){
                    has_error=1;
                    ++error_cnt;
                    const int temp_buf_size = 32;
                    char buf1[temp_buf_size];
                    char buf2[temp_buf_size];
                    get_type_name($1, buf1, temp_buf_size);
                    get_type_name($3, buf2, temp_buf_size);
                    fprintf(stderr, "Line %d, at char %d: Type mismatch (left is %s but right is %s), cannot substract different types.\n", @2.first_line, @2.first_column, buf1, buf2);
                    $$ = TYPE_UNKNOWN;
                }else if($1 == TYPE_STRING){
                    has_error=1;
                    ++error_cnt;
                    fprintf(stderr, "Line %d, at char %d: Cannot perform arithmetic on strings.\n", @2.first_line, @2.first_column);
                    $$ = TYPE_UNKNOWN;
                }else{
                    $$ = $1; 
                }
            }
            ;

term        : factor { $$ = $1; }
            | term '*' factor {
                if($1 != $3 && $1 != TYPE_UNKNOWN && $3 != TYPE_UNKNOWN){
                    has_error=1;
                    ++error_cnt;
                    const int temp_buf_size = 32;
                    char buf1[temp_buf_size];
                    char buf2[temp_buf_size];
                    get_type_name($1, buf1, temp_buf_size);
                    get_type_name($3, buf2, temp_buf_size);
                    fprintf(stderr, "Line %d, at char %d: Type mismatch (left is %s but right is %s), cannot mul different types.\n", @2.first_line, @2.first_column, buf1, buf2);
                    $$ = TYPE_UNKNOWN;
                }else if($1 == TYPE_STRING){
                    has_error=1;
                    ++error_cnt;
                    fprintf(stderr, "Line %d, at char %d: Cannot perform arithmetic on strings.\n", @2.first_line, @2.first_column);
                    $$ = TYPE_UNKNOWN;
                }else{
                    $$ = $1; 
                }
            }
            | term '/' factor { $$ = TYPE_REAL; }
            | term R_DIV factor { $$ = TYPE_INT; }
            | term R_MOD factor { $$ = TYPE_INT; }
            ;

factor      : varid  { $$ = $1; }
            | INT    { $$ = TYPE_INT; }
            | REAL   { $$ = TYPE_REAL; }
            | STRING { $$ = TYPE_STRING; free($1); }
            | '(' exp ')' { $$ = $2; }
            | '-' factor %prec UMINUS { $$ = $2; }
            | '+' factor %prec UPLUS { $$ = $2; }
            ;

call_func   : ID '(' exp_list ')' { free($1); }
            ;

exp_list    : exp 
            | exp_list ',' exp 
            ;

for_stmt    : R_FOR index_exp R_DO body
            ;

index_exp   : ID S_ASSIGN exp R_TO exp {
                DataType t = get_type($1, @1.first_line, @1.first_column);
                free($1);
            }
            /*| ID S_ASSIGN exp error {
                fprintf(stderr, "Line %d, at char %d: expected 'to' after expression.\n", @4.first_line, @4.first_column);
            }*/
            ;

varid       : ID                { $$ = get_type($1, @1.first_line, @1.first_column); free($1); }
            | ID '[' exp ']'    { $$ = get_type($1, @1.first_line, @1.first_column); free($1); }
            ;

body        : stmt
            | R_BEGIN stmt_list R_END
            ;

%%

int main(){
    yyparse();
    if(error_cnt){
        printf("[Compiler] Failed! %d errors were found!\n", error_cnt);
    }
    return 0;
}


void yyerror(const char *s){
    has_error = 1;
    ++error_cnt;
    fprintf(stderr, "Line %d, at char %d: %s\n", line_cnt, char_cnt-yyleng, s);
}

void add_symbol(char* name, DataType type){
    for(int i = 0; i < sym_count; i++){
        if(strcmp(sym_table[i].name, name) == 0){
            has_error=1;
            ++error_cnt;
            fprintf(stderr, "Line %d: Variable '%s' already declared.\n", line_cnt, name);
            return;
        }
    }
    strcpy(sym_table[sym_count].name, name);
    sym_table[sym_count].type = type;
    sym_count++;
}

DataType get_type(char* name, int line, int col){
    for(int i = 0; i < sym_count; i++){
        if(strcmp(sym_table[i].name, name) == 0){
            return sym_table[i].type;
        }
    }

    has_error=1;
    ++error_cnt;
    fprintf(stderr, "Line %d, at char %d: Undefined variable '%s'\n", line, col, name);
    return TYPE_UNKNOWN;
}

// typedef enum { TYPE_INT, TYPE_REAL, TYPE_STRING, TYPE_UNKNOWN } DataType;
void get_type_name(DataType dt, char* buf, const int buf_size){
    if(dt == TYPE_INT){
        snprintf(buf, buf_size, "Integer");
    }else if(dt == TYPE_REAL){
        snprintf(buf, buf_size, "Real");
    }else if(dt == TYPE_STRING){
        snprintf(buf, buf_size, "String");
    }else{
        snprintf(buf, buf_size, "Unknown");
    }
}