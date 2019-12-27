// Import the libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Define an enumeration to list all the possible token types
enum type {operator,
           separator,
           identifier,
           number,
           keyword,
           string};
// Define an enumeration to list all the possible instruction types
enum instruction_type{function,
                      function_call,
                      znumber,
                      zbool,
                      zstring,
                      vardeclaration,
                      ifstatement,
                      whilestatement,
                      returnstatement,
                      AST,
                      zidentifier};
// Define the structure for a token with a type and a name
struct token {
    enum type type;
    char      value[50];
};
// Define the structure for a list of tokens
struct lexline {
    struct token *tokens;
    int           size;
    int           base_value;
};
// Define the structure for a node of an AST
struct leaf {
    enum instruction_type type; // Declaration of the type of the node
    union {                     // Store every possible node in an memory spot
        struct whilestatement       *ast_while;
        struct ifstatement          *ast_if;
        struct leaf                 *ast;
        struct functioncall         *ast_function;
        struct number               *ast_number;
        struct string               *ast_string;
        struct variable_declaration *ast_vardeclaration;
        struct identifier           *ast_identifier;
    };
};
// Define the structures for the different AST node types
struct whilestatement{
    struct leaf *condition;
    struct leaf *body;
};
struct ifstatement{
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct functioncall{
    int body_length;
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
struct identifier {
    char name[30];
};
struct parse {
    struct leaf *body;
    int size;
};
char symbols[10] = {'(',')','{','}',';','"','[',']',',','#'}; // List of all symbols
char operators[9] = {'>','<', '=', '?', '+', '/', '-', '%','^'}; // List of all operators
char *keywords[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"}; // List of keybords
int linum = 1; // The variable to keep track of the current line
FILE *fp1;
// A function to check if a string is a keyword
int iskeyword(char in[]){
    for(int i = 0; i < 12; i++){
        if (strcmp(keywords[i], in) == 0){
            return 1;
        }
    }
    return 0;
}
// A function to check if a char is in a char list
int isinchars(char in[], char check){
    for(int i = 0; i < 11; i++){
        if (in[i] == check){
            return 1;
        }
    }
    return 0;
}
// A function to check the operator precedence of an input string
int operatorPrecedence (char operator[]){
    int precedence = -1; // If the input is not an operator or is empty, return -1
    if (((strcmp(operator, "and")) == 0) ||
        ((strcmp(operator,  "or")) == 0) ||
        ((strcmp(operator, "and")) == 0) ||
        ((strcmp(operator,  "==")) == 0) ||
        ((strcmp(operator,   "<")) == 0) ||
        ((strcmp(operator,   ">")) == 0) ||
        ((strcmp(operator,  ">=")) == 0) ||
        ((strcmp(operator,  "<=")) == 0) ||
        ((strcmp(operator,   "?")) == 0))
    {precedence = 0;}
    else if (((strcmp(operator, "+")) == 0) ||
             ((strcmp(operator, "-")) == 0))
    {precedence = 1;}
    else if (((strcmp(operator, "*")) == 0) ||
             ((strcmp(operator, "/")) == 0) ||
             ((strcmp(operator, "%")) == 0))
    {precedence = 2;}
    else if (strcmp(operator, "^") == 0)
    {precedence = 3;}
    else if (strcmp(operator, ".") == 0)
    {precedence = 4;}
    else if (strcmp(operator, "=") == 0)
    {precedence = 5;}
    return precedence;
}
// The Lexer
struct lexline lexer(FILE *fp1, char breaker){
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
            while ((c != ' ') && (c != '\n') && (!isinchars(symbols, c)) && (!isinchars(operators, c))){
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
            else if ((c == '/') && (d == '/')){
                while (c != '\n'){
                    c = fgetc(fp1);
                }
                k --;
            }
            else{
                conv[0] = c;
                strcpy((tokens+k)->value, conv);
                fseek(fp1, ftell(fp1)-1, SEEK_SET);
            }
            k ++;
        }
        else if (isinchars(symbols, c)){
            if (c == '"'){
                j = ftell(fp1);
                i = 0;
                c = fgetc(fp1);
                while (c != '"'){
                    i++;
                    c = fgetc(fp1);
                }
                l = ftell(fp1);
                fseek(fp1, j, SEEK_SET);
                fgets(buffer, i + 1, fp1);
                fseek(fp1, l, SEEK_SET);
                (tokens+k)->type = 5;
                strcpy((tokens+k)->value, buffer);
            }
            else if (c == '#'){
                c = fgetc(fp1);
                while (c != '#'){
                    c = fgetc(fp1);
                }
                k --;
                c = fgetc(fp1);
            }
            else{
                (tokens+k)->type = 1;
                conv[0] = c;
                strcpy((tokens+k)->value, conv);
            }
            k++;
        }
        else if (c == '\t'){
            (tokens + k) -> type = 1;
            strcpy((tokens+k)->value, "tabulation");
            k ++;
        }
        if (c == breaker){
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

void tabulation(int tabs){
    for(int i = 0; i < tabs; i++){
        printf("  ");
    }
    return;
}

void printAST(struct leaf *AST, int tabs){
    tabulation(tabs);
    switch(AST -> type){
        case 0:
            break;
        case 1:
            printf("function : %s \n", AST -> ast_function -> function);
            tabulation(tabs + 1);
            printf("length : %d\n", AST -> ast_function -> body_length);
            for (int i = 0; i < (AST -> ast_function ) -> body_length; i++){
                tabulation(tabs + 1);
                printf("argument n°%d : ",i);
                printAST(AST -> ast_function -> body, 0);
                AST->ast_function -> body ++;
            }
            break;
        case 2:
            printf("number : %i\n", AST -> ast_number-> value);
            break;
        case 3:
            break;
        case 4:
            printf("string : \"%s\"\n", AST -> ast_string -> value);
            break;
        case 5:
            printf("variable declaration : %s \n", AST -> ast_vardeclaration -> name);
            break;
        case 6:
            printf("if : \n");
            tabulation(tabs + 1);
            printf("condition : \n");
            printAST(AST -> ast_if -> condition, tabs + 2);
            tabulation(tabs + 1);
            printf("body : \n");
            for (int i = 0; i < AST -> ast_if -> body_length; i++){
                printAST(AST -> ast_if -> body, tabs + 2);
                AST -> ast_if -> body ++;
            }
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            printAST(AST -> ast, tabs + 1);
            break;
        case 10:
            printf("identifier : %s\n", AST -> ast_identifier -> name);
            break;
    }
}

void freeall(struct leaf *AST){
    switch(AST -> type){
        case 0:
            break;
        case 1:
            for (int i = 0; i < (AST -> ast_function ) -> body_length; i++){
                freeall(AST -> ast_function -> body);
                AST->ast_function -> body ++;
            }
            free(AST -> ast_function);
            break;
        case 2:
            free(AST -> ast_number);
            break;
        case 3:
            break;
        case 4:
            free(AST -> ast_string);
            break;
        case 5:
            free(AST -> ast_vardeclaration);
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            freeall(AST -> ast);
            free(AST -> ast);
            break;
        case 10:
            free(AST -> ast_identifier);
            break;
    }
    free(AST);
}

void copy_ast(struct leaf *transmitter, struct leaf *receiver, int index1, int index2){
    transmitter += index1;
    receiver += index2;
    switch(transmitter -> type){
        case 0:
            break;
        case 1:
            receiver -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
            strcpy(receiver -> ast_function -> function, transmitter -> ast_function -> function);
            receiver -> ast_function -> body = (struct leaf*) malloc((transmitter -> ast_function -> body_length) * sizeof(struct leaf));
            receiver -> ast_function -> body_length = transmitter -> ast_function -> body_length;
            for (int i = 0; i < (transmitter -> ast_function -> body_length); i++){
                copy_ast(transmitter -> ast_function -> body, receiver -> ast_function -> body, i, i);
            }
            break;
        case 2:
            receiver -> ast_number = (struct number*) malloc(sizeof(struct number));
            receiver -> ast_number -> value = transmitter -> ast_number -> value;
            break;
        case 3:
            break;
        case 4:
            receiver -> ast_string = (struct string*) malloc(sizeof(struct string));
            strcpy(receiver -> ast_string -> value, transmitter -> ast_string -> value);
            break;
        case 5:
            receiver -> ast_vardeclaration = (struct variable_declaration*) malloc(sizeof(struct variable_declaration));
            strcpy(receiver -> ast_vardeclaration -> name, transmitter -> ast_vardeclaration -> name);
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            receiver -> ast = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast, transmitter -> ast, 0, 0);
            break;
        case 10:
            receiver -> ast_identifier = (struct identifier*) malloc(sizeof(struct identifier));
            strcpy(receiver -> ast_identifier -> name, transmitter -> ast_identifier -> name);
            break;
    }
    receiver -> type = transmitter -> type;
    transmitter -= index1;
    receiver -= index2;
}

struct parse parsestatement(struct lexline lex, char terminator2[20]){
    struct leaf *arg2;
    struct leaf *Ast;
    struct token *tokens = lex.tokens;
    struct token operators[5];
    Ast = (struct leaf*) malloc((lex.size - lex.base_value) * sizeof(struct leaf));
    int aindex = 0;
    int size = lex.base_value;
    int current_operator = 0;
    while(size <= lex.size){
        struct token token;
        token.type = (tokens+size)->type;
        strcpy(token.value, (tokens+size)->value);
        if (strcmp(token.value, terminator2) == 0){
            break;
        }
        else if (token.type == 3){
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
            while (current_operator >= 0){
                struct token operator = operators[current_operator];
                if (operatorPrecedence(token.value) <= operatorPrecedence(operator.value)){
                    (Ast + aindex - 2) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                    (Ast + aindex - 2) -> ast_function -> body  = (struct leaf*) malloc(2 * sizeof(struct leaf));
                    (Ast + aindex - 2) -> type = 1;
                    strcpy(((Ast + aindex - 2) -> ast_function) -> function, operator.value);
                    (Ast + aindex - 2) -> ast_function -> body = Ast + aindex - 1;
                    (((Ast + aindex - 2) -> ast_function) -> body + 1) -> type = arg2 -> type;
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
        else if (token.type == 1){
            if(token.value[1] == '('){
                size ++;
                lex.base_value = size;
                (Ast + aindex) -> type = 9;
                (Ast + aindex) -> ast  = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(parsestatement(lex, "terminator").body, (Ast + aindex) -> ast, 0, 0);
                aindex ++;
            }
            else if (')'){
                break;
            }
            else if ('{'){
                break;
            }
        }
        else if (token.type == 4){
            if (strcmp(token.value, "let") == 0){
                (Ast + aindex) -> type = 5;
                (Ast + aindex) -> ast_vardeclaration = (struct variable_declaration*) malloc(sizeof(struct variable_declaration));
                strcpy((Ast + aindex) -> ast_vardeclaration -> name, (tokens + size + 1) -> value);
                aindex ++;
            }
            else if (strcmp(token.value, "if") == 0){
                (Ast + aindex) -> ast_if = (struct ifstatement*) malloc(sizeof(struct ifstatement));
                lex.base_value = size + 1;
                struct parse argcondition;
                struct parse argbody;
                argcondition = parsestatement(lex, "terminator");
                argbody = parsestatement(lexer(fp1, '}'), "terminator");
                (Ast + aindex) -> type = 6;
                (Ast + aindex) -> ast_if -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_if -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_if -> body, i, i);
                }
                (Ast + aindex) -> ast_if -> body_length = argbody.size;
                aindex ++;
                while(((tokens + size) -> value)[0] != '{' ){
                    size ++;
                }
            }
            else if (strcmp(token.value, "print") == 0){
                (Ast + aindex) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                lex.base_value = size + 1;
                arg2 = (struct leaf*) malloc(sizeof(struct leaf));
                arg2 = parsestatement(lex, "terminator").body;
                (Ast + aindex) -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(arg2, ((Ast + aindex) -> ast_function -> body), 0, 0);
                (Ast + aindex) -> type = 1;
                (Ast -> ast_function ) -> body_length = 1;
                strcpy((Ast + aindex) -> ast_function -> function, "print");
                aindex ++;
                size ++;
            }
            size ++;
        }
        else if (token.type == 2){
            (Ast + aindex) -> type = 10;
            (Ast + aindex) -> ast_identifier = (struct identifier*) malloc(sizeof(struct identifier));
            strcpy((Ast + aindex) -> ast_identifier -> name, token.value);
            aindex ++;
            size ++;
        }
    }
    current_operator --;
    while(current_operator >= 0){
        arg2 = (struct leaf*) malloc(sizeof(struct leaf));
        copy_ast(Ast, arg2, aindex - 2, 0);
        struct token operator = operators[current_operator];
        (Ast + aindex - 2) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
        (Ast + aindex - 2) -> ast_function -> body = (struct leaf*) malloc(2 * sizeof(struct leaf));
        (Ast + aindex - 2) -> type = 1;
        strcpy(((Ast + aindex - 2) -> ast_function) -> function, operators[current_operator].value);
        copy_ast(arg2, (Ast + aindex - 2) -> ast_function -> body, 0, 0);
        copy_ast(Ast, (Ast + aindex - 2) -> ast_function -> body, aindex - 1, 1);
        ((Ast + aindex - 2) -> ast_function ) -> body_length = 2;
        aindex --;
        current_operator --;
    }
    struct parse output;
    output.body = Ast;
    output.size = aindex;
    return(output);
    free(tokens);
    freeall(Ast);
    free(Ast);
}

int main( int argc, char *argv[] ){
    fp1 = fopen (argv[1], "r");
    while(1){
        printAST(parsestatement(lexer(fp1, '\n'), "terminator").body, 0);
        linum ++;
    }
    fclose(fp1);
    return 0;
}
