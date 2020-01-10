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
           string,
           switch_indent};
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
                      zidentifier,
                      elsestatement,
                      elifstatement};
enum variable_type{integer,
                   char_list,
                   bool};
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
        struct function             *ast_functiondeclaration;
        struct functioncall         *ast_function;
        struct number               *ast_number;
        struct string               *ast_string;
        struct variable_declaration *ast_vardeclaration;
        struct identifier           *ast_identifier;
        struct bool                 *ast_bool;
        struct elsestatement        *ast_else;
        struct elifstatement        *ast_elif;
    };
};
// Define the structures for the different AST node types
struct function{
    char         name[50];
    int          body_length;
    int          argnumber;
    char         arguments[5][10];
    struct leaf *body;
};
struct whilestatement{
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct ifstatement{
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct elifstatement{
    int          truth;
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct elsestatement{
    int          truth;
    int          body_length;
    struct leaf *body;
};
struct functioncall{
    int          body_length;
    char         function[15];
    struct leaf *body;
};
struct number{
    int value;
};
struct bool {
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
struct variable{
    char               name[20];
    enum variable_type type;
    union {
        char string[50];
        int  integer;
    };
};
char opps[11] = {'>','<', '=', '!', '+', '/', '-', '%','^', '*','<'}; // List of all operators
char symbols[11] = {'(',')','{','}',';','"','[',']',',','#','\n'}; // List of all symbols
char *keywords[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"}; // List of keybords
int linum = 1; // The variable to keep track of the current line
FILE *fp1;
struct variable *symbol_table;
int varind = 0;
int symbol_table_length = 20;
int actualindentlevel = 0;
int actualspaces = 0;

// A function to check if a string is a keyword
int iskeyword(char in[]){
    for(int i = 0; i < 11; i++){
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
    {precedence = -1;}
    return precedence;
}
int varindex (char var[]){
    int j = -1;
    for (int i = 0; i < symbol_table_length; i++){
        if (strcmp((symbol_table + i) -> name, var) == 0){
            j = i;
        }
    }
    if (j == -1){
        puts("Error : using non declarated variable.");
        exit(1);
    }
    return j;
}
// The Lexer
struct lexline lexer(FILE *fp1, int min_indent){
    struct token *tokens;
    struct lexline lex;
    tokens = (struct token*) malloc(20 * sizeof(struct token));
    char c = fgetc(fp1);
    fseek(fp1, -1, SEEK_CUR);
    char d = ' ';
    int i = 0, j = 0, k = 0, l = 0;
    char buffer[60];
    char conv[2] = {'a', '\0'};
    while(c != EOF){
        c = fgetc(fp1);
        if (c == EOF){
            break;
        }

        if (isalpha(c)){
            j = ftell(fp1);
            i = 0;
            while ((c != ' ') && (c != '\n') && (!isinchars(symbols, c)) && (!isinchars(opps, c))){
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
            while ((c != ' ') && (c != '\n') && (!isinchars(symbols, c)) && (!isinchars(opps, c))){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(buffer, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            (tokens+k) -> type = 3;
            strcpy((tokens+k)->value, buffer);
            k++;
        }
        else if (isinchars(opps, c)){
            (tokens+k)->type = 0;
            d = fgetc(fp1);
            if ((c == '=') && (d == '=')){
                strcpy((tokens + k)->value, "==");
            }
            else if ((c == '<') && (d == '=')){
                strcpy((tokens + k)->value, "<=");
            }
            else if ((c == '>') && (d == '=')){
                strcpy((tokens + k)->value, ">=");
            }
            else if ((c == '/') && (d == '/')){
                while (c != '\n'){
                    c = fgetc(fp1);
                }
                linum ++;
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
                    if (c == '\n'){
                        linum ++;
                    }
                    c = fgetc(fp1);
                }
                k --;
                c = fgetc(fp1);
            }
            else{
                (tokens+k)->type = 1;
                conv[0] = c;
                strcpy((tokens+k)->value, conv);
                if(c == '\n'){
                    int indent = 0;
                    c = fgetc(fp1);
                    while (c == ' '){
                        c = fgetc(fp1);
                        indent ++;
                    }
                    if ( c != EOF ){
                        fseek(fp1, ftell(fp1) - 1, SEEK_SET);
                    }
                    indent = indent / 2;
                    if (indent <= min_indent){
                        lex.size = k;
                        lex.base_value = 0;
                        lex.tokens = tokens;
                        return lex;
                    }
                    linum ++;
                }
            }
            k++;
        }
    }
    lex.size = -1;
    return lex;
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
            printf("declare function : %s \n", AST -> ast_functiondeclaration -> name);
            tabulation(tabs + 1);
            for (int i = 0; i < AST -> ast_functiondeclaration -> argnumber; i ++) {
                printf("%s ", AST -> ast_functiondeclaration -> arguments[i]);
            }
            printf("\n");
            tabulation(tabs + 1);
            puts("body :");
            for (int i = 0; i < AST -> ast_functiondeclaration -> body_length; i++){
                printAST(AST -> ast_functiondeclaration -> body, tabs + 2);
                AST -> ast_functiondeclaration -> body ++;
            }
            AST -> ast_functiondeclaration -> body -= AST -> ast_functiondeclaration -> body_length;
            break;
        case 1:
            printf("function : %s \n", AST -> ast_function -> function);
            for (int i = 0; i < (AST -> ast_function ) -> body_length; i++){
                tabulation(tabs + 1);
                printf("argument n°%d : \n",i);
                printAST(AST -> ast_function -> body, tabs + 2);
                AST->ast_function -> body ++;
            }
            AST->ast_function -> body -= (AST -> ast_function ) -> body_length;
            break;
        case 2:
            printf("number : %i\n", AST -> ast_number-> value);
            break;
        case 3:
            printf("bool : ");
            if (AST -> ast_bool -> value == 0){
                printf("False\n");
            }
            else {
                printf("True\n");
            }
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
            AST -> ast_if -> body -= AST -> ast_if -> body_length;
            break;
        case 7:
            printf("while : \n");
            tabulation(tabs + 1);
            printf("condition : \n");
            printAST(AST -> ast_while -> condition, tabs + 2);
            tabulation(tabs + 1);
            printf("body : \n");
            for (int i = 0; i < AST -> ast_while -> body_length; i++){
                printAST(AST -> ast_while -> body, tabs + 2);
                AST -> ast_while -> body ++;
            }
            AST -> ast_while -> body -= AST -> ast_while -> body_length;
            break;
        case 8:
            break;
        case 9:
            printAST(AST -> ast, tabs + 1);
            break;
        case 10:
            printf("identifier : %s\n", AST -> ast_identifier -> name);
            break;
        case 11:
            printf("else : \n");
            tabulation(tabs + 1);
            printf("body : \n");
            for (int i = 0; i < AST -> ast_else -> body_length; i++){
                printAST(AST -> ast_else -> body, tabs + 2);
                AST -> ast_else -> body ++;
            }
            AST -> ast_else -> body -= AST -> ast_else -> body_length;
            break;
        case 12:
            printf("elif : \n");
            tabulation(tabs + 1);
            printf("condition : \n");
            printAST(AST -> ast_elif -> condition, tabs + 2);
            tabulation(tabs + 1);
            for (int i = 0; i < AST -> ast_elif -> body_length; i++){
                printAST(AST -> ast_elif -> body, tabs + 2);
                AST -> ast_else -> body ++;
            }
            AST -> ast_elif -> body -= AST -> ast_elif -> body_length;
    }
}

void freeall(struct leaf *AST){
    switch(AST -> type){
        case 0:
            for (int i = 0; i < AST -> ast_functiondeclaration -> body_length; i++){
                freeall(AST -> ast_functiondeclaration -> body);
                AST -> ast_functiondeclaration -> body ++;
            }
            break;
        case 1:
            for (int i = 0; i < AST -> ast_function -> body_length; i++){
                freeall(AST -> ast_function -> body);
                AST -> ast_function -> body ++;
            }
            free(AST -> ast_function);
            break;
        case 2:
            free(AST -> ast_number);
            break;
        case 3:
            free(AST -> ast_bool);
            break;
        case 4:
            free(AST -> ast_string);
            break;
        case 5:
            free(AST -> ast_vardeclaration);
            break;
        case 6:
            for (int i = 0; i < AST -> ast_if -> body_length; i++){
                freeall(AST -> ast_if -> body);
                AST -> ast_if -> body ++;
            }
            freeall(AST -> ast_if -> condition);
            free(AST -> ast_if);
            break;
        case 7:
            for (int i = 0; i < AST -> ast_while -> body_length; i++){
                freeall(AST -> ast_while -> body);
                AST -> ast_while -> body ++;
            }
            freeall(AST -> ast_while -> condition);
            free(AST -> ast_while);
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
        case 11:
            for (int i = 0; i < AST -> ast_else -> body_length; i++){
                freeall(AST -> ast_else -> body);
                AST -> ast_else -> body ++;
            }
            free(AST -> ast_else);
            break;
        case 12:
            for (int i = 0; i < AST -> ast_elif -> body_length; i++){
                freeall(AST -> ast_elif -> body);
                AST -> ast_elif -> body ++;
            }
            freeall(AST -> ast_elif -> condition);
            free(AST -> ast_elif);
    }
}

void copy_ast(struct leaf *transmitter, struct leaf *receiver, int index1, int index2){
    transmitter += index1;
    receiver += index2;
    switch(transmitter -> type){
        case 0:
            receiver -> ast_functiondeclaration = (struct function*) malloc(sizeof(struct function));
            strcpy(receiver -> ast_functiondeclaration -> name, transmitter -> ast_functiondeclaration -> name);
            receiver -> ast_functiondeclaration -> argnumber = transmitter -> ast_functiondeclaration -> argnumber;
            receiver -> ast_functiondeclaration -> body_length = transmitter -> ast_functiondeclaration -> body_length;
            for (int i = 0; i < transmitter -> ast_functiondeclaration -> argnumber; i++){
                strcpy(receiver -> ast_functiondeclaration -> arguments[i], transmitter -> ast_functiondeclaration -> arguments[i]);
            }
            for (int i = 0; i < (transmitter -> ast_functiondeclaration -> body_length); i++){
                copy_ast(transmitter -> ast_functiondeclaration -> body, receiver -> ast_functiondeclaration -> body, i, i);
            }
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
            receiver -> ast_bool = (struct bool*) malloc(sizeof(struct bool));
            receiver -> ast_bool -> value = transmitter -> ast_bool -> value;
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
            receiver -> ast_if = (struct ifstatement*) malloc(sizeof(struct ifstatement));
            receiver -> ast_if -> body_length = transmitter -> ast_if -> body_length;
            receiver -> ast_if -> body = (struct leaf*) malloc((transmitter -> ast_if -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_if -> body_length); i++){
                copy_ast(transmitter -> ast_if -> body, receiver -> ast_if -> body, i, i);
            }
            receiver -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_if -> condition, receiver -> ast_if -> condition, 0, 0);
            break;
        case 7:
            receiver -> ast_while = (struct whilestatement*) malloc(sizeof(struct whilestatement));
            receiver -> ast_while -> body_length = transmitter -> ast_while -> body_length;
            receiver -> ast_while -> body = (struct leaf*) malloc((transmitter -> ast_while -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_while -> body_length); i++){
                copy_ast(transmitter -> ast_while -> body, receiver -> ast_while -> body, i, i);
            }
            receiver -> ast_while -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_while -> condition, receiver -> ast_while -> condition, 0, 0);
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
        case 11:
            receiver -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
            receiver -> ast_else -> body_length = transmitter -> ast_else -> body_length;
            receiver -> ast_else -> body = (struct leaf*) malloc((transmitter -> ast_else -> body_length) * sizeof(struct leaf));
            receiver -> ast_else -> truth = transmitter -> ast_else -> truth;
            for (int i = 0; i < (transmitter -> ast_else -> body_length); i++){
                copy_ast(transmitter -> ast_else -> body, receiver -> ast_else -> body, i, i);
            }
            break;
        case 12:
            receiver -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
            receiver -> ast_elif -> body_length = transmitter -> ast_elif -> body_length;
            receiver -> ast_elif -> body = (struct leaf*) malloc((transmitter -> ast_elif -> body_length) * sizeof(struct leaf));
            receiver -> ast_elif -> truth = transmitter -> ast_elif -> truth;
            for (int i = 0; i < (transmitter -> ast_elif -> body_length); i++){
                copy_ast(transmitter -> ast_elif -> body, receiver -> ast_elif -> body, i, i);
            }
            receiver -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_elif -> condition, receiver -> ast_while -> condition, 0, 0);
    }
    receiver -> type = transmitter -> type;
}

struct parse parsestatement(struct lexline lex, char terminator2[20]){
    struct leaf *arg2;
    struct leaf *Ast;
    struct token *tokens = lex.tokens;
    struct token operators[5];
    for (int i = 0; i < 5; i++) {
        strcpy(operators[i].value, " ");
    }
    Ast = (struct leaf*) malloc((lex.size - lex.base_value) * sizeof(struct leaf));
    int aindex = 0;
    int size = lex.base_value;
    int current_operator = 0;
    if (lex.size == -1){
        struct parse output;
        output.size = -1;
        free(Ast);
        return(output);
    }
    while(size <= lex.size){
        struct token token;
        token.type = (tokens+size)->type;
        strcpy(token.value, (tokens+size)->value);
        if ((strcmp(token.value, terminator2) == 0)){
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
            while (current_operator > 0){
                if (operatorPrecedence(token.value) <= operatorPrecedence(operators[current_operator - 1].value)){
                    arg2 = (struct leaf*) malloc(sizeof(struct leaf));
                    copy_ast(Ast, arg2, aindex - 2, 0);
                    (Ast + aindex - 2) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                    (Ast + aindex - 2) -> ast_function -> body = (struct leaf*) malloc(2 * sizeof(struct leaf));
                    (Ast + aindex - 2) -> type = 1;
                    strcpy(((Ast + aindex - 2) -> ast_function) -> function, operators[current_operator - 1].value);
                    copy_ast(arg2, (Ast + aindex - 2) -> ast_function -> body, 0, 0);
                    copy_ast(Ast, (Ast + aindex - 2) -> ast_function -> body, aindex - 1, 1);
                    ((Ast + aindex - 2) -> ast_function ) -> body_length = 2;
                    aindex --;
                    current_operator --;
                }
                else{
                    break;
                }
            }
            strcpy(operators[current_operator].value, token.value);
            current_operator ++;
            size ++;
        }
        else if (token.type == 1){
            if(token.value[0] == '('){
                size ++;
                lex.base_value = size;
                copy_ast(parsestatement(lex, ")").body, (Ast + aindex), 0, 0);
                while (strcmp((tokens + size) -> value, ")")){
                    size ++;
                }
                size ++;
                aindex ++;
            }
            else if (token.value[0] == '\n'){
                break;
            }
        }
        else if (token.type == 4){
            if (strcmp(token.value, "let") == 0){
                if ((((tokens + size + 2) -> value) [0] != '=') && (((tokens + size + 2) -> value) [0] != '\n')){
                    (Ast + aindex) -> type = 0;
                    (Ast + aindex) -> ast_functiondeclaration = (struct function*) malloc(sizeof(struct function));
                    strcpy((Ast + aindex) -> ast_functiondeclaration -> name,
                           (tokens + size + 1) -> value);
                    size += 3;
                    while ((tokens + size) -> type == 1){
                        strcpy((Ast + aindex) -> ast_functiondeclaration -> arguments[(Ast + aindex) -> ast_functiondeclaration -> argnumber],
                               (tokens + size) -> value);
                        (Ast + aindex) -> ast_functiondeclaration -> argnumber ++;
                        size ++;
                    }
                    size += 4;
                    (Ast + aindex) -> ast_functiondeclaration -> argnumber --;
                    struct parse body = parsestatement(lexer(fp1, '}'), "}");
                    (Ast + aindex) -> ast_functiondeclaration -> body_length = body.size;
                    (Ast + aindex) -> ast_functiondeclaration -> body = (struct leaf*) malloc(sizeof(struct leaf));
                    for (int i = 0; i < body.size; i ++){
                        copy_ast(body.body, (Ast + aindex) -> ast_functiondeclaration -> body, 0, 0);
                        (Ast + aindex) -> ast_functiondeclaration -> body ++;
                        body.body ++;
                    }
                    (Ast + aindex) -> ast_functiondeclaration -> body -= body.size;
                }
                else {
                    (Ast + aindex) -> type = 5;
                    (Ast + aindex) -> ast_vardeclaration = (struct variable_declaration *)malloc(sizeof(struct variable_declaration));
                    strcpy((Ast + aindex) -> ast_vardeclaration -> name, (tokens + size + 1) -> value);
                }
                aindex ++;
            }
            else if (strcmp(token.value, "if") == 0){
                (Ast + aindex) -> ast_if = (struct ifstatement*) malloc(sizeof(struct ifstatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n");
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "}");
                (Ast + aindex) -> type = 6;
                (Ast + aindex) -> ast_if -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_if -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_if -> body, 0, 0);
                    (Ast + aindex) -> ast_if -> body ++;
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_if -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_if -> body -= argbody.size;
                aindex ++;
                break;
            }
            else if (strcmp(token.value, "while") == 0){
                (Ast + aindex) -> ast_while = (struct whilestatement*) malloc(sizeof(struct whilestatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n");
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "}");
                (Ast + aindex) -> type = 7;
                (Ast + aindex) -> ast_while -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_while -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_while -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_while -> body, 0, 0);
                    (Ast + aindex) -> ast_while -> body ++;
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_while -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_while -> body -= argbody.size;
                aindex ++;
                break;
            }
            else if (strcmp(token.value, "else") == 0){
                (Ast + aindex) -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "}");
                lex.base_value = size + 1;
                (Ast + aindex) -> type = 11;
                (Ast + aindex) -> ast_else -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_else -> truth = 1;
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_else -> body, 0, 0);
                    (Ast + aindex) -> ast_else -> body ++;
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_else -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_else -> body -= argbody.size;
                aindex ++;
                break;
            }
            else if (strcmp(token.value, "elif") == 0){
                (Ast + aindex) -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n");
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "}");
                (Ast + aindex) -> type = 12;
                (Ast + aindex) -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_elif -> condition, 0, 0);
                (Ast + aindex) -> ast_elif -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_elif -> truth = 1;
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_elif -> body, 0, 0);
                    (Ast + aindex) -> ast_elif -> body ++;
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_elif -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_elif -> body -= argbody.size;
                aindex ++;
                break;
            }
            else if (strcmp(token.value, "print") == 0){
                (Ast + aindex) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                lex.base_value = size + 1;
                size ++;
                arg2 = parsestatement(lex, "\n").body;
                (Ast + aindex) -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(arg2, ((Ast + aindex) -> ast_function -> body), 0, 0);
                (Ast + aindex) -> type = 1;
                ((Ast + aindex) -> ast_function ) -> body_length = 1;
                strcpy((Ast + aindex) -> ast_function -> function, "print");
                while (((tokens + size) -> value)[0] != '\n'){
                    size ++;
                }
                aindex ++;
                break;
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
    if(isinchars(opps, operators[current_operator].value[0])){
        while (current_operator >= 0) {
            arg2 = (struct leaf *)malloc(sizeof(struct leaf));
            copy_ast(Ast, arg2, aindex - 2, 0);
            (Ast + aindex - 2)->ast_function = (struct functioncall *)malloc(sizeof(struct functioncall));
            (Ast + aindex - 2)->ast_function->body = (struct leaf *)malloc(2 * sizeof(struct leaf));
            (Ast + aindex - 2)->type = 1;
            strcpy(((Ast + aindex - 2)->ast_function)->function,
                   operators[current_operator].value);
            copy_ast(arg2, (Ast + aindex - 2)->ast_function->body, 0, 0);
            copy_ast(Ast, (Ast + aindex - 2)->ast_function->body, aindex - 1, 1);
            ((Ast + aindex - 2)->ast_function)->body_length = 2;
            aindex --;
            current_operator--;
        }
    }
    struct parse output;
    output.body = Ast;
    output.size = aindex;
    return(output);
}

void check(struct leaf *Ast){
    if (Ast -> type == 1){
        if (strcmp(Ast -> ast_function -> function, "=") == 0){
            if (Ast -> ast_function -> body -> type != 10){
                puts("Error : assigning a value to a non variable element.");
                exit(1);
            }
        }
    }
    else if (Ast -> type == 11) {
        if (((Ast - 1) -> type != 6) && ((Ast - 1) -> type != 12)){
            puts("Error : using an else without an if.");
            exit(1);
        }
    }
    return;
}

void execute(struct leaf *Ast){
    switch (Ast -> type){
        case 5 :
            strcpy((symbol_table + varind) -> name, Ast -> ast_vardeclaration -> name);
            (symbol_table + varind) -> type = -1;
            varind ++;
            break;
        case 1 :
            if (strcmp(Ast -> ast_function -> function, "=") == 0){
                int j = varindex(Ast -> ast_function -> body -> ast_identifier -> name);
                Ast -> ast_function -> body ++;
                if (Ast -> ast_function -> body -> type == 1){
                    execute(Ast -> ast_function -> body);
                }
                Ast -> ast_function -> body --;
                for (int u = 0; u < Ast -> ast_function -> body_length; u ++){
                    if (Ast -> ast_function -> body -> type == 10) {
                        int j = varindex(Ast -> ast_function -> body -> ast_identifier -> name);
                        if ((symbol_table + j) -> type == 0){
                            free(Ast -> ast_function -> body -> ast_identifier);
                            Ast -> ast_function -> body -> type = 2;
                            Ast -> ast_function -> body -> ast_number = (struct number*) malloc(sizeof(struct number));
                            Ast -> ast_function -> body -> ast_number -> value = (symbol_table + j) -> integer;
                        }
                    }
                    Ast -> ast_function -> body ++;
                }
                Ast -> ast_function -> body --;
                if (Ast -> ast_function -> body -> type == 2){
                    if (((symbol_table + j) -> type == 0) || ((symbol_table + j) -> type == -1)){
                        (symbol_table + j) -> type = 0;
                        (symbol_table + j) -> integer = Ast -> ast_function -> body -> ast_number -> value;
                    }
                    else {
                        puts("Error : changing the type of a variable");
                        exit(1);
                    }
                    Ast -> ast_function -> body --;
                    return;
                }
                else if(Ast -> ast_function -> body -> type == 4){
                    if (((symbol_table + j) -> type == 1) || ((symbol_table + j) -> type == -1)){
                        (symbol_table + j) -> type = 1;
                        strcpy((symbol_table + j) -> string, Ast -> ast_function -> body -> ast_string -> value);
                    }
                    else {
                        puts("Error : changing the type of a variable");
                        exit(1);
                    }
                    Ast -> ast_function -> body --;
                    return;
                }
            }
            else if ((strcmp(Ast -> ast_function -> function, "+") == 0) ||
                 (strcmp(Ast -> ast_function -> function, "-") == 0) ||
                 (strcmp(Ast -> ast_function -> function, "*") == 0) ||
                 (strcmp(Ast -> ast_function -> function, "/") == 0)){
                for (int u = 0; u < Ast -> ast_function -> body_length; u ++){
                    if (Ast -> ast_function -> body -> type == 10) {
                        int j = varindex(Ast -> ast_function -> body -> ast_identifier -> name);
                        if ((symbol_table + j) -> type == 0){
                            Ast -> ast_function -> body -> type = 2;
                            Ast -> ast_function -> body -> ast_number = (struct number*) malloc(sizeof(struct number));
                            Ast -> ast_function -> body -> ast_number -> value = (symbol_table + j) -> integer;
                        }
                    }
                    else if (Ast -> ast_function -> body -> type == 1) {
                        execute(Ast -> ast_function -> body);
                    }
                    Ast -> ast_function -> body ++;
                }
                Ast -> ast_function -> body -= Ast -> ast_function -> body_length;
                if (Ast -> ast_function -> body -> type == 2){
                    int i = Ast -> ast_function -> body -> ast_number -> value;
                    Ast -> ast_function -> body ++;
                    if (Ast -> ast_function -> body -> type == 2){
                        int j = Ast -> ast_function -> body -> ast_number -> value;
                        Ast -> type = 2;
                        char c = (Ast -> ast_function -> function)[0];
                        Ast -> ast_number = (struct number*) malloc(sizeof(struct number));
                        switch(c){
                            case '+':
                                Ast -> ast_number -> value = i + j;
                                break;
                            case '-':
                                Ast -> ast_number -> value = i - j;
                                break;
                            case '*':
                                Ast -> ast_number -> value = i * j;
                                break;
                            case '/':
                                Ast -> ast_number -> value = i / j;
                                break;
                        }
                    }
                }
            }
            else if ((strcmp(Ast -> ast_function -> function, "<") == 0) ||
                     (strcmp(Ast -> ast_function -> function, ">") == 0) ||
                     (strcmp(Ast -> ast_function -> function, "==") == 0)){
                for (int u = 0; u < 2; u ++){
                    if (Ast -> ast_function -> body -> type == 10) {
                        int j = varindex(Ast -> ast_function -> body -> ast_identifier -> name);
                        if ((symbol_table + j) -> type == 0){
                            Ast -> ast_function -> body -> type = 2;
                            Ast -> ast_function -> body -> ast_number = (struct number*) malloc(sizeof(struct number));
                            Ast -> ast_function -> body -> ast_number -> value = (symbol_table + j) -> integer;
                        }
                    }
                    else if (Ast -> ast_function -> body -> type == 1) {
                        execute(Ast -> ast_function -> body);
                    }
                    Ast -> ast_function -> body ++;
                }
                Ast -> ast_function -> body -= 2;
                int t1 = Ast -> ast_function -> body -> type;
                Ast -> ast_function -> body ++;
                int t2 = Ast -> ast_function -> body -> type;
                if ((t1 == t2) && (t1== 2)){
                    int j = Ast -> ast_function -> body -> ast_number -> value;
                    Ast -> ast_function -> body --;
                    int i = Ast -> ast_function -> body -> ast_number -> value;
                    if (((i < j)  && strcmp(Ast -> ast_function -> function, "<") == 0) ||
                        ((i > j)  && strcmp(Ast -> ast_function -> function, ">") == 0) ||
                        ((i == j) && strcmp(Ast ->ast_function -> function, "==") == 0)){
                        Ast -> type = 3;
                        Ast -> ast_bool = (struct bool*) malloc (sizeof(struct bool));
                        Ast -> ast_bool -> value = 1;
                    }
                    else {
                        Ast -> type = 3;
                        Ast -> ast_bool = (struct bool*) malloc (sizeof(struct bool));
                        Ast -> ast_bool -> value = 0;
                    }
                }
                else if ((t1 == t2) && (t1 == 4)) {
                    char j[100];
                    strcpy(j, Ast -> ast_function -> body -> ast_string -> value);
                    Ast -> ast_function -> body ++;
                    char i[100];
                    strcpy(i, Ast -> ast_function -> body -> ast_string -> value);
                    if ((strcmp(Ast -> ast_function -> function, "<") == 0) ||
                        (strcmp(Ast -> ast_function -> function, ">") == 0)){
                        puts("Error : can't say if a string is superior or inferior to another string.");
                    }
                    else if (strcmp(i, j) == 0){
                        Ast -> type = 3;
                        Ast -> ast_bool = (struct bool*) malloc (sizeof(struct bool));
                        Ast -> ast_bool -> value = 1;
                    }
                    else  {
                        Ast -> type = 3;
                        Ast -> ast_bool = (struct bool*) malloc (sizeof(struct bool));
                        Ast -> ast_bool -> value = 0;
                    }
                }
                else {
                    puts("Error : comparing values of different types.");
                    exit(0);
                }
            }
            else if (strcmp(Ast -> ast_function -> function, "print") == 0){
                if (Ast -> ast_function -> body -> type == 1){
                    execute(Ast -> ast_function -> body);
                }
                if (Ast -> ast_function -> body -> type == 4){
                    puts(Ast -> ast_function -> body -> ast_string -> value);
                }
                else if (Ast -> ast_function -> body -> type == 10){
                    int j = varindex(Ast -> ast_function -> body -> ast_identifier -> name);
                    if ((symbol_table + j) -> type == 0) {
                        printf("%d\n", (symbol_table + j) -> integer);
                    }
                    else if ((symbol_table + j) -> type == 1){
                        puts((symbol_table + j) -> string);
                    }
                }
            }
            break;
        case 6 :
            if (Ast -> ast_if -> condition -> type == 1){
                execute(Ast -> ast_if -> condition);
            }
            if (Ast -> ast_if -> condition -> type == 3){
                if (Ast -> ast_if -> condition -> ast_bool -> value == 1){
                    if ((Ast + 1) -> type == 11) {
                        (Ast + 1) -> ast_else -> truth = 0;
                    }
                    else if ((Ast + 1) -> type == 12) {
                        (Ast + 1) -> ast_elif -> truth = 0;
                    }
                    for (int u = 0; u < Ast -> ast_if -> body_length; u ++){
                        execute(Ast -> ast_if -> body);
                        Ast -> ast_if -> body ++;
                    }
                    Ast -> ast_if -> body -= Ast -> ast_if -> body_length;
                }
                else {
                    if ((Ast + 1) -> type == 11) {
                        (Ast + 1) -> ast_else -> truth = 1;
                    }
                    else if ((Ast + 1) -> type == 12) {
                        (Ast + 1) -> ast_elif -> truth = 1;
                    }
                }
            }
            break;
        case 7 : {
            struct leaf *Ast1 = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(Ast -> ast_while -> condition, Ast1, 0, 0);
            if (Ast -> ast_while -> condition -> type == 1){
                execute(Ast1);
            }
            if (Ast1 -> type == 3){
                while(1){
                    free(Ast1 -> ast_bool);
                    copy_ast(Ast -> ast_while -> condition, Ast1, 0, 0);
                    if (Ast1 -> type == 1){
                        execute(Ast1);
                    }
                    if (Ast1 -> ast_bool -> value == 1){
                        for (int u = 0; u < Ast -> ast_while -> body_length; u ++){
                            struct leaf *Ast2;
                            Ast2 = (struct leaf*) malloc(sizeof(struct leaf));
                            copy_ast(Ast -> ast_while -> body, Ast2, 0, 0);
                            execute(Ast2);
                            Ast -> ast_while -> body ++;
                        }
                        Ast -> ast_while -> body -= Ast -> ast_while -> body_length;
                    }
                    else {
                        break;
                    }
                }
            }
            break;
        }
        case 11:
            if (Ast -> ast_else -> truth == 1) {
                for (int u = 0; u < Ast -> ast_else -> body_length; u ++){
                    execute(Ast -> ast_else -> body);
                    Ast -> ast_else -> body ++;
                }
                Ast -> ast_else -> body -= Ast -> ast_else -> body_length;
            }
            break;
        case 12 :
            if (Ast -> ast_elif -> truth == 1){
                if(Ast -> ast_elif -> condition -> type == 1){
                    execute(Ast -> ast_elif -> condition);
                }
                if (Ast -> ast_elif -> condition -> type == 3){
                    if (Ast -> ast_elif -> condition -> ast_bool -> value == 1){
                        if ((Ast + 1) -> type == 11) {
                            (Ast + 1) -> ast_else -> truth = 0;
                        }
                        for (int u = 0; u < Ast -> ast_if -> body_length; u ++){
                            execute(Ast -> ast_if -> body);
                            Ast -> ast_if -> body ++;
                        }
                        Ast -> ast_if -> body -= Ast -> ast_if -> body_length;
                    }
                    else {
                        if ((Ast + 1) -> type == 11) {
                            (Ast + 1) -> ast_else -> truth = 1;
                        }
                        else if ((Ast + 1) -> type == 12) {
                            (Ast + 1) -> ast_elif -> truth = 1;
                        }
                    }
                }
            }
            else {
                if ((Ast + 1) -> type == 11) {
                    (Ast + 1) -> ast_else -> truth = 0;
                }
                else if ((Ast + 1) -> type == 12) {
                    (Ast + 1) -> ast_elif -> truth = 0;
                }
            }
            break;
    }
}

int main( int argc, char *argv[] ){
    if (argc != 2){
        exit(1);
    }
    fp1 = fopen (argv[1], "r");
    struct parse outfinal;
    symbol_table  = (struct variable*) malloc(20 * sizeof(struct variable));
    outfinal.size = 0;
    outfinal.body = (struct leaf*) malloc(10 * sizeof(struct leaf));
    while(1){
        struct parse out = parsestatement(lexer(fp1, 0), "\n");
        for (int i = 0; i < out.size; i ++){
            copy_ast(out.body, outfinal.body, 0, outfinal.size);
            outfinal.size ++;
            out.body ++;
        }
        if (out.size == -1){
            break;
        }
    }
    fclose(fp1);
    for (int i = 0; i < outfinal.size; i++){
        check(outfinal.body);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    for (int i = 0; i < outfinal.size; i++){
        execute(outfinal.body);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    for (int i = 0; i < outfinal.size; i++){
        freeall(outfinal.body);
        outfinal.body ++;
    }
    exit(0);
    return 0;
}
