#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum type {operator,
           separator,
           identifier,
           number,
           keyword,
           string};
enum instruction_type{function,
                      function_call,
                      znumber,
                      zbool,
                      zstring,
                      vardeclaration,
                      ifstatement,
                      whilestatement,
                      returnstatement,
                      AST};
struct token {
    enum type type;
    char value[50];
};
struct lexline {
    struct token *tokens;
    int size;
    int base_value;
};
struct leaf {
    enum instruction_type type;
    union {
        struct whilestatement       *ast_while;
        struct leaf                 *ast;
        struct functioncall         *ast_function;
        struct number               *ast_number;
        struct string               *ast_string;
        struct variable_declaration *ast_vardeclaration;
    };
};
struct whilestatement{
    struct leaf *condition;
    struct leaf *body;
};
struct functioncall{
    char function[15];
    struct leaf *body;
};
struct number{
    int value;
};
struct string{
    char value[100];
};
struct variable_declaration {
    char name[30];
};
char symbols[10] = {'(',')','{','}',';','"','[',']',',','#'};
char operators[9] = {'>','<', '=', '?', '+', '/', '-', '%','^'};
char *keywords[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"};
int linum = 1;
int varind = 0;

int iskeyword(char in[]){
    for(int i = 0; i < 12; i++){
        if (strcmp(keywords[i], in) == 0){
            return 1;
        }
    }
    return 0;
}

int isinchars(char in[], char check){
    for(int i = 0; i < 10; i++){
        if (in[i] == check){
            return 1;
        }
    }
    return 0;
}

int operatorPrecedence (char operator[]){
    int precedence = -1;
    if (((strcmp(operator, "and")) == 0) ||
        ((strcmp(operator, "or")) == 0) ||
        ((strcmp(operator, "and")) == 0) ||
        ((strcmp(operator, "==")) == 0) ||
        ((strcmp(operator, "<")) == 0) ||
        ((strcmp(operator, ">")) == 0) ||
        ((strcmp(operator, ">=")) == 0) ||
        ((strcmp(operator, "<=")) == 0) ||
        ((strcmp(operator, "?")) == 0))
    {precedence = 0;}
    else if (((strcmp(operator, "+")) == 0) ||
             ((strcmp(operator, "-")) == 0))
    {precedence = 1;}
    else if (((strcmp(operator, "*")) == 0) ||
             ((strcmp(operator, "/")) == 0))
    {precedence = 2;}
    else if (strcmp(operator, "^") == 0)
    {precedence = 3;}
    else if (strcmp(operator, ".") == 0)
    {precedence = 4;}
    else if (strcmp(operator, "=") == 0)
    {precedence = 5;}
    return precedence;
}

struct lexline lexer(FILE *fp1){
    struct token *tokens;
    struct lexline lex;
    tokens = (struct token*) malloc(20 * sizeof(struct token));
    char c = ' ';
    char d = ' ';
    int i = 0, j = 0, k = 0, l = 0;
    char buffer[60];
    char conv[2] = {'a', '\0'};
    while(c != EOF){
        c = fgetc(fp1);
        if (isalpha(c)){
            j = ftell(fp1);
            i = 0;
            while ((c != ' ') && (c != '\n')){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(buffer, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            if (iskeyword(buffer)){
                (tokens+k)->type = 4;
            }
            else{
                (tokens+k)->type = 2;
            }
            strcpy((tokens+k)->value, buffer);
            k++;
        }
        else if (isdigit(c)){
            j = ftell(fp1);
            i = 0;
            while ((c != ' ') && (c != '\n')){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(buffer, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            (tokens+k)->type = 3;
            strcpy((tokens+k)->value, buffer);
            puts((tokens+k)->value);
            k++;
        }
        else if (isinchars(operators, c)){
            (tokens+k)->type = 0;
            d = fgetc(fp1);
            if ((c == '=') && (d == '=')){
                strcpy((tokens+k)->value, "==");
            }
            else if ((c == '<') && (d == '=')){
                strcpy((tokens+k)->value, "<=");
            }
            else if ((c == '>') && (d == '=')){
                strcpy((tokens+k)->value, ">=");
            }
            else{
                conv[0] = c;
                strcpy((tokens+k)->value, conv);
                fseek(fp1, ftell(fp1)-1, SEEK_SET);
            }
            k++;
        }
        else if (isinchars(symbols, c)){
            if (c == '"'){
                j = ftell(fp1);
                i = 0;
                while (c != '"'){
                    i++;
                    c = fgetc(fp1);
                }
                l = ftell(fp1);
                fseek(fp1, j - 1, SEEK_SET);
                fgets(buffer, i + 1, fp1);
                fseek(fp1, l - 1, SEEK_SET);
                (tokens+k)->type = 5;
            }
            else{
                (tokens+k)->type = 1;
                conv[0] = c;
                strcpy((tokens+k)->value, conv);
            }
            k++;
        }
        else if (c == '\n'){
            (tokens+k)->type = 1;
            strcpy((tokens+k)->value, "terminator");
            tokens = (struct token*) realloc(tokens, (k + 1) * sizeof(struct token));
            lex.size = (k + 1);
            lex.base_value = 0;
            lex.tokens=tokens;
            return lex;
            free(tokens);
        }
    }
    exit(0);
}

struct leaf * parsestatement(struct lexline lex){
    struct token *tokens = lex.tokens;
    struct leaf *Ast;
    struct token operators[5];
    Ast = (struct leaf*) malloc(lex.size * sizeof(struct leaf));
    int aindex = 0;
    int size = lex.base_value;
    int current_operator = 0;
    while(size <= lex.size){
        puts((tokens+size)->value);
        struct token token;
        token.type = (tokens+size)->type;
        strcpy(token.value, (tokens+size)->value);
        if (token.type == 3){
            (Ast + aindex) -> ast_number = (struct number*) malloc(sizeof(struct number));
            (Ast + aindex) -> type = 2;
            ((Ast + aindex) -> ast_number) -> value = atoi(token.value);
            aindex ++;
            size ++;
        }
        else if (token.type == 5){
            (Ast+aindex) -> ast_string = (struct string*) malloc(sizeof(struct string));
            (Ast+aindex)->type = 4;
            strcpy(((Ast + aindex) -> ast_string) -> value, token.value);
            aindex ++;
            size ++;
        }
        else if (token.type == 0){
            while (current_operator > 0){
                struct token operator = operators[current_operator];
                if (operatorPrecedence(token.value) <= operatorPrecedence(operator.value)){
                    struct leaf *arg2 = Ast + aindex - 2;
                    (Ast + aindex - 2) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                    (Ast + aindex - 2) -> ast_function -> body  = (struct leaf*) malloc(2 * sizeof(struct leaf));
                    (Ast + aindex - 2) -> type = 1;
                    strcpy(((Ast + aindex - 2) -> ast_function) -> function, operator.value);
                    (Ast + aindex - 2) -> ast_function -> body = Ast + aindex - 1;
                    (Ast + aindex - 2) -> ast_function -> body = arg2;
                    aindex --;
                    current_operator --;
                }
                else{
                    break;
                }
            }
            operators[current_operator] = token;
            current_operator ++;
            size ++;
        }
        if (token.type == 1){
            if(token.value[1] == '('){
                size ++;
                lex.base_value = size;
                (Ast + aindex) -> ast = parsestatement(lex);
                size ++;
                aindex ++;
            }
            else if (')'){
                break;
            }
        }
        else if (strcmp(token.value, "terminator") == 0){
            break;
        }
        else if (token.type == 4){
            size ++;
        }
        else if (token.type == 2){
            size ++;
        }
    }
    while(current_operator > 0){
        printf("%d",aindex);
        struct leaf *arg2 = Ast + aindex - 2;
        (Ast + aindex - 2) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
        (Ast + aindex - 2) -> ast_function -> body  = (struct leaf*) malloc(2 * sizeof(struct leaf));
        (Ast + aindex - 2) -> type = 1;
        strcpy(((Ast + aindex - 2) -> ast_function) -> function, operators[current_operator].value);
        (Ast + aindex - 2) -> ast_function -> body = Ast + aindex - 1;
        (Ast + aindex - 2) -> ast_function -> body = arg2;
        aindex --;
        current_operator --;
    }
    printf("%d", Ast -> type);
    return(Ast);
    free(tokens);
    free(Ast);
}

int main( int argc, char *argv[] ){
    FILE *fp1 = fopen (argv[1], "r");
    while(1){
        parsestatement(lexer(fp1));
        linum ++;
    }
    fclose(fp1);
    return 0;
}
