// Import the libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Define an enumeration to list all the possible token types
enum type { operator,
            separator,
            identifier,
            number,
            keyword,
            string };
// Define an enumeration to list all the possible instruction types
enum instruction_type{ function,
                       function_call,
                       znumber,
                       zbool,
                       zstring,
                       vardeclaration,
                       ifstatement,
                       whilestatement,
                       zidentifier,
                       elsestatement,
                       elifstatement,
                       ast};
enum variable_type{ integer,
                    char_list,
                    bool,
                    function_decl};
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
    int length;
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
    char         name[30];
    int          has_index;
    struct leaf *index;
};
struct parse {
    struct leaf *body;
    int size;
};
struct variable{
    char               name[20];
    enum variable_type type;
    int                is_static;
    int                array_length;
    union {
        char        string[50];
        int         integer;
        struct leaf *ast;
    };
};
char opps[11] = {'>','<', '=', '!', '+', '/', '-', '%','^', '*','<'}; // List of all operators
char symbols[11] = {'(',')','{','}',';','"','[',']',',','#','\n'}; // List of all symbols
char *keywords[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"}; // List of keybords
int linum = 1; // The variable to keep track of the current line
FILE *fp1;
FILE *outfile;
struct variable *symbol_table;
int varind = 0; // Keep track of the actual free symbol table place
int actualindentlevel = 0;
int file_length = 0;

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
        ((strcmp(operator,  "!=")) == 0) ||
        ((strcmp(operator,   "<")) == 0) ||
        ((strcmp(operator,   ">")) == 0) ||
        ((strcmp(operator,  ">=")) == 0) ||
        ((strcmp(operator,  "<=")) == 0) ||
        ((strcmp(operator,   "?")) == 0))
        precedence = 0;
    else if (((strcmp(operator, "+")) == 0) ||
             ((strcmp(operator, "-")) == 0))
        precedence = 1;
    else if (((strcmp(operator, "*")) == 0) ||
             ((strcmp(operator, "/")) == 0) ||
             ((strcmp(operator, "%")) == 0))
        precedence = 2;
    else if (strcmp(operator, "^") == 0)
        precedence = 3;
    else if (strcmp(operator, ".") == 0)
        precedence = 4;
    else if (strcmp(operator, "=") == 0)
        precedence = -1;
    return precedence;
}

int varindex (char var[]){
    for (int i = 0; i < 20; i++){
        if (strcmp((symbol_table + i) -> name, var) == 0){
            return i;
        }
    }
    puts("Error : using non declarated variable.");
    exit(1);
}
// The Lexer
struct lexline lexer(FILE *fp1, int min_indent, struct token *tokens){
    struct lexline lex;
    char c = ' ';
    char d = ' ';
    int i = 0, j = 0, k = 0, l = 0;
    char buffer[60];
    char conv[2] = {'a', '\0'};
    while(ftell(fp1) < file_length){
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
                strcpy((tokens + k) -> value, "==");
            }
            else if ((c == '<') && (d == '=')){
                strcpy((tokens + k) -> value, "<=");
            }
            else if ((c == '>') && (d == '=')){
                strcpy((tokens + k) -> value, ">=");
            }
            else if ((c == '!') && (d == '=')){
                strcpy((tokens + k) -> value, "!=");
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
                (tokens + k) -> type = 5;
                strcpy ((tokens + k) -> value, buffer);
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
            } else {
                (tokens + k) -> type = 1;
                conv[0] = c;
                strcpy((tokens + k) -> value, conv);
                if(c == '\n'){
                    int indent = 0;
                    c = fgetc(fp1);
                    while (c == ' '){
                        c = fgetc(fp1);
                        indent ++;
                    }
                    if (c != EOF){
                        fseek(fp1, ftell(fp1) - 1, SEEK_SET);
                    }
                    indent = indent / 2;
                    if (indent < actualindentlevel){
                        k ++;
                        actualindentlevel = indent;
                        strcpy((tokens + k) -> value, "switch_indent");
                    }
                    if (indent <= min_indent){
                        lex.size = k - 1;
                        lex.base_value = 0;
                        lex.tokens = tokens;
                        return lex;
                    }
                    actualindentlevel = indent;
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
                printf ("argument n°%d : \n",i);
                printAST (AST -> ast_function -> body, tabs + 2);
                AST -> ast_function -> body ++;
            }
            AST -> ast_function -> body -= (AST -> ast_function ) -> body_length;
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
            printf("identifier : %s\n", AST -> ast_identifier -> name);
            break;
        case 9:
            printf("else : \n");
            tabulation(tabs + 1);
            printf("body : \n");
            for (int i = 0; i < AST -> ast_else -> body_length; i++){
                printAST(AST -> ast_else -> body, tabs + 2);
                AST -> ast_else -> body ++;
            }
            AST -> ast_else -> body -= AST -> ast_else -> body_length;
            break;
        case 10:
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
            break;
        case 11:
        {
            int j = AST -> ast -> length;
            for (int i = 0; i < j; i++)
            {

                printAST (AST -> ast, 0);
                AST -> ast ++;
            }
            AST -> ast -= j;
            break;
        }
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
            AST -> ast_function -> body -= AST -> ast_function -> body_length;
            free(AST -> ast_function -> body);
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
            AST -> ast_if -> body -= AST -> ast_if -> body_length;
            free(AST -> ast_if -> body);
            freeall(AST -> ast_if -> condition);
            free(AST -> ast_if -> condition);
            free(AST -> ast_if);
            break;
        case 7:
            for (int i = 0; i < AST -> ast_while -> body_length; i++){
                freeall(AST -> ast_while -> body);
                AST -> ast_while -> body ++;
            }
            free(AST -> ast_while -> condition);
            freeall(AST -> ast_while -> condition);
            free(AST -> ast_while);
            break;
        case 8:
            if (1 == AST -> ast_identifier -> has_index)
            {
                freeall(AST -> ast_identifier -> index);
                free(AST -> ast_identifier -> index);
            }
            free(AST -> ast_identifier);
            break;
        case 9:
            for (int i = 0; i < AST -> ast_else -> body_length; i++){
                freeall(AST -> ast_else -> body);
                AST -> ast_else -> body ++;
            }
            AST -> ast_else -> body -= AST -> ast_else -> body_length;
            free(AST -> ast_else -> body);
            free(AST -> ast_else);
            break;
        case 10 :
            for (int i = 0; i < AST -> ast_elif -> body_length; i++){
                freeall(AST -> ast_elif -> body);
                AST -> ast_elif -> body ++;
            }
            freeall(AST -> ast_elif -> condition);
            free(AST -> ast_elif);
            break;
        case 11:
            for (int i = 0; i < AST -> ast -> length; i++){
                freeall(AST -> ast);
                AST -> ast ++;
            }
            break;
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
            receiver -> ast_identifier = (struct identifier*) malloc(sizeof(struct identifier));
            receiver -> ast_identifier -> has_index = transmitter -> ast_identifier -> has_index;
            if (1 == transmitter -> ast_identifier -> has_index)
            {
                receiver -> ast_identifier -> index = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(transmitter -> ast_identifier -> index, receiver -> ast_identifier -> index, 0, 0);
            }
            strcpy (receiver -> ast_identifier -> name, transmitter -> ast_identifier -> name);
            break;
        case 9:
            receiver -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
            receiver -> ast_else -> body_length = transmitter -> ast_else -> body_length;
            receiver -> ast_else -> body = (struct leaf*) malloc((transmitter -> ast_else -> body_length) * sizeof(struct leaf));
            receiver -> ast_else -> truth = transmitter -> ast_else -> truth;
            for (int i = 0; i < (transmitter -> ast_else -> body_length); i++){
                copy_ast(transmitter -> ast_else -> body, receiver -> ast_else -> body, i, i);
            }
            break;
        case 10:
            receiver -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
            receiver -> ast_elif -> body_length = transmitter -> ast_elif -> body_length;
            receiver -> ast_elif -> body = (struct leaf*) malloc((transmitter -> ast_elif -> body_length) * sizeof(struct leaf));
            receiver -> ast_elif -> truth = transmitter -> ast_elif -> truth;
            for (int i = 0; i < (transmitter -> ast_elif -> body_length); i++){
                copy_ast(transmitter -> ast_elif -> body, receiver -> ast_elif -> body, i, i);
            }
            receiver -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_elif -> condition, receiver -> ast_while -> condition, 0, 0);
            break;
        case 11:
        {
            int j = transmitter -> ast -> length;
            receiver -> ast = (struct leaf*) malloc(j * sizeof(struct leaf));
            receiver -> ast -> length = j;
            for (int i = 0; i < j; i++){
                copy_ast(transmitter -> ast, receiver -> ast, i, i);
            }
            break;
        }
    }
    receiver -> type = transmitter -> type;
}

struct parse parsestatement(struct lexline lex, char terminator2[20]){
    if (lex.size == -1){
        struct parse output;
        output.size = -1;
        return(output);
    }
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

    while (size <= lex.size){
        struct token token;
        token.type = (tokens + size) -> type;
        strcpy(token.value, (tokens + size) -> value);
        if ((strcmp(token.value, terminator2) == 0)){
            break;
        }
        if (token.type == 3){
            (Ast + aindex) -> length = 1;
            (Ast + aindex) -> ast_number = (struct number*) malloc(sizeof(struct number));
            (Ast + aindex) -> type = 2;
            (Ast + aindex) -> ast_number -> value = atoi(token.value);
            aindex ++;
            size ++;
        }
        else if (token.type == 5){
            (Ast + aindex) -> ast_string = (struct string*) malloc(sizeof(struct string));
            (Ast + aindex) -> type = 4;
            strcpy (((Ast + aindex) -> ast_string) -> value, token.value);
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
                else {
                    break;
                }
            }
            strcpy(operators[current_operator].value, token.value);
            current_operator ++;
            size ++;
        }
        else if (token.type == 1){
            if (token.value[0] == '('){
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
                if (current_operator > 0){
                    current_operator --;
                }
                if (isinchars(opps, operators[current_operator].value[0])){
                    while (current_operator >= 0) {
                        arg2 = (struct leaf *) malloc(sizeof(struct leaf));
                        copy_ast(Ast, arg2, aindex - 2, 0);
                        (Ast + aindex - 2) -> ast_function = (struct functioncall *) malloc(sizeof(struct functioncall));
                        (Ast + aindex - 2) -> ast_function->body = (struct leaf *) malloc(2 * sizeof(struct leaf));
                        (Ast + aindex - 2) -> type = 1;
                        strcpy(((Ast + aindex - 2) -> ast_function) -> function,
                               operators[current_operator].value);
                        copy_ast(arg2, (Ast + aindex - 2)->ast_function->body, 0, 0);
                        copy_ast(Ast, (Ast + aindex - 2)->ast_function->body, aindex - 1, 1);
                        ((Ast + aindex - 2)->ast_function)->body_length = 2;
                        aindex --;
                        current_operator--;
                    }
                }
                size ++;
            }
            else if (token.value[0] == '{')
            {
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "}");
                (Ast + aindex) -> type = 11;
                (Ast + aindex) -> ast = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast, i, i);
                }
                (Ast + aindex) -> ast -> length = argbody.size;
                aindex ++;
                while ((strcmp("}", (tokens + size) -> value)) && (size <= lex.size)){
                    size ++;
                }
            }
            else if (!strcmp("switch_indent", token.value))
            {
                size ++;
            }
            else if (token.value[0] == '}')
            {
                    size ++;
            }
            else if (token.value[0] == '[')
            {
                size ++;
                int arraysize = 0;
                while ((tokens + size) -> value[0] != ']')
                {
                    if ((tokens + size) -> type == 3)
                    {
                        arraysize ++;
                    }
                    size ++;
                }
                size -= arraysize;
                (Ast + aindex) -> type = 2;
                (Ast + aindex) -> ast_number = (struct number*) malloc(arraysize * sizeof(struct number));
                (Ast + aindex) -> length = arraysize;
                while ((tokens + size) -> value[0] != ']')
                {
                    if ((tokens + size) -> type == 3)
                    {
                        (Ast + aindex) -> ast_number -> value = atoi((tokens + size) -> value);
                        (Ast + aindex) -> ast_number ++;
                    }
                    size ++;
                }
                (Ast + aindex) -> ast_number -= arraysize;
                size ++;
                aindex ++;
            }
        }
        else if (token.type == 4){
            if (strcmp(token.value, "let") == 0){
                if ((((tokens + size + 2) -> value) [0] != '=') && (((tokens + size + 2) -> value) [0] != '[')){
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
                struct parse argbody = parsestatement(lex, "switch_indent");
                (Ast + aindex) -> type = 6;
                (Ast + aindex) -> ast_if -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_if -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_if -> body, 0, 0);
                    (Ast + aindex) -> ast_if -> body ++;
                    freeall(argbody.body);
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_if -> body_length = argbody.size;
                argbody.body -= argbody.size;
                free(argbody.body);
                (Ast + aindex) -> ast_if -> body -= argbody.size;
                aindex ++;
                while (strcmp("switch_indent", (tokens + size) -> value)){
                    size ++;
                }
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else if (strcmp(token.value, "while") == 0){
                (Ast + aindex) -> ast_while = (struct whilestatement*) malloc(sizeof(struct whilestatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n");
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent");
                (Ast + aindex) -> type = 7;
                (Ast + aindex) -> ast_while -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_while -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_while -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_while -> body, 0, 0);
                    (Ast + aindex) -> ast_while -> body ++;
                    freeall(argbody.body);
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_while -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_while -> body -= argbody.size;
                aindex ++;
                while (strcmp("switch_indent", (tokens + size) -> value)){
                    size ++;
                }
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else if (strcmp(token.value, "else") == 0){
                (Ast + aindex) -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent");
                (Ast + aindex) -> type = 9;
                (Ast + aindex) -> ast_else -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_else -> truth = 1;
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_else -> body, 0, 0);
                    (Ast + aindex) -> ast_else -> body ++;
                    freeall(argbody.body);
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_else -> body_length = argbody.size;
                argbody.body -= argbody.size;
                free(argbody.body);
                (Ast + aindex) -> ast_else -> body -= argbody.size;
                aindex ++;
                while (strcmp("switch_indent", (tokens + size) -> value)){
                    size ++;
                }
            }
            else if (strcmp(token.value, "elif") == 0){
                (Ast + aindex) -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n");
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent");
                (Ast + aindex) -> type = 10;
                (Ast + aindex) -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_elif -> condition, 0, 0);
                (Ast + aindex) -> ast_elif -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_elif -> truth = 1;
                for (int i = 0; i < argbody.size; i ++){
                    copy_ast(argbody.body, (Ast + aindex) -> ast_elif -> body, 0, 0);
                    (Ast + aindex) -> ast_elif -> body ++;
                    freeall(argbody.body);
                    argbody.body ++;
                }
                (Ast + aindex) -> ast_elif -> body_length = argbody.size;
                argbody.body -= argbody.size;
                (Ast + aindex) -> ast_elif -> body -= argbody.size;
                aindex ++;
                while (strcmp("switch_indent", (tokens + size) -> value)){
                    size ++;
                }
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else if (strcmp(token.value, "print") == 0){
                (Ast + aindex) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                size ++;
                lex.base_value = size;
                arg2 = parsestatement(lex, "\n").body;
                (Ast + aindex) -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(arg2, ((Ast + aindex) -> ast_function -> body), 0, 0);
                (Ast + aindex) -> type = 1;
                ((Ast + aindex) -> ast_function ) -> body_length = 1;
                strcpy((Ast + aindex) -> ast_function -> function, "print");
                while (((tokens + size) -> value)[0] != '\n'){
                    size ++;
                }
                freeall(arg2);
                free(arg2);
                aindex ++;
            }
            size ++;
        }
        else if (token.type == 2){
            (Ast + aindex) -> type = 8;
            (Ast + aindex) -> ast_identifier = (struct identifier*) malloc (sizeof(struct identifier));
            strcpy((Ast + aindex) -> ast_identifier -> name, token.value);
            size ++;
            if ((tokens + size) -> value[0] == '[')
            {
                lex.base_value = size + 1;
                struct parse argbody = parsestatement (lex, "]");
                (Ast + aindex) -> ast_identifier -> index = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_identifier -> has_index = 1;
                copy_ast(argbody.body, (Ast + aindex) -> ast_identifier -> index, 0, 0);
                while (((tokens + size) -> value)[0] != ']'){
                    size ++;
                }
                freeall(argbody.body);
                free(argbody.body);
                size ++;
            }
            else
            {
                (Ast + aindex) -> ast_identifier -> has_index = 0;
            }
            aindex ++;
        }
    }
    if (current_operator > 0){
        current_operator --;
    }
    if (isinchars(opps, operators[current_operator].value[0])){
        while (current_operator >= 0) {
            arg2 = (struct leaf *) malloc(sizeof(struct leaf));
            copy_ast(Ast, arg2, aindex - 2, 0);
            freeall(Ast + aindex - 2);
            (Ast + aindex - 2) -> ast_function = (struct functioncall *) malloc(sizeof(struct functioncall));
            (Ast + aindex - 2) -> ast_function -> body = (struct leaf *) malloc(2 * sizeof(struct leaf));
            (Ast + aindex - 2) -> type = 1;
            strcpy(((Ast + aindex - 2) -> ast_function) -> function,
                   operators[current_operator].value);
            copy_ast(arg2, (Ast + aindex - 2) -> ast_function->body, 0, 0);
            copy_ast(Ast, (Ast + aindex - 2) -> ast_function->body, aindex - 1, 1);
            ((Ast + aindex - 2) -> ast_function) -> body_length = 2;
            freeall(Ast + aindex - 1);
            freeall(arg2);
            free(arg2);
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
            if (Ast -> ast_function -> body -> type != 8){
                puts("Error : assigning a value to a non variable element.");
                exit(1);
            }
        }
    }
    else if (Ast -> type == 9) {
        if (((Ast - 1) -> type != 6) && ((Ast - 1) -> type != 10)){
            puts("Error : using an else without an if.");
            exit(1);
        }
    }
    return;
}

// Functions for the actual compiler
struct reg // the structure of a register and its name
{
    char              *name;
    enum variable_type type;
};

static char *reglist[4] = { "r8", "r9", "r10", "r11" }; //List of registers
int number_stings = 0;
int used_registers = 0;
int number_functions = 0;
int used_functions[3] = {0, 0, 0};
int number_cmp = 0;
int nubmer_structures = 0;

int new_register (void)
{
    used_registers ++;
    return used_registers - 1;
}

void free_register (void){
    used_registers --;
    return;
}

struct reg compile (struct leaf *Ast)
{
    struct reg outreg;
    outreg.name = malloc (sizeof(*outreg.name) * 256);

    outreg.type = -1;
    switch (Ast -> type)
    {
        case 0 :
            break;
        case 1 :
            switch (Ast -> ast_function -> function[0])
            {
                case '-' :
                case '+' :
                {
                    char *arg1 = malloc (sizeof(*arg1) * 256);
                    strcpy(arg1, compile (Ast -> ast_function -> body).name);
                    Ast -> ast_function -> body ++;
                    char *arg2 = malloc (sizeof(*arg2) * 256);
                    strcpy(arg2, compile (Ast -> ast_function -> body).name);
                    Ast -> ast_function -> body --;
                    if (Ast -> ast_function -> function[0] == '+')
                    {
                        fprintf(outfile, "\tadd\t%s, %s\n", arg1, arg2);
                    }
                    else
                    {
                        fprintf(outfile, "\tsub\t%s, %s\n", arg1, arg2);
                    }
                    free_register();
                    free(arg2);
                    outreg.type = 0;
                    strcpy (outreg.name, arg1);
                    return outreg;
                }
                case '/' :
                case '*' :
                    fprintf(outfile, "\tmov\trax, %d\n", Ast -> ast_function -> body -> ast_number -> value);
                    Ast -> ast_function -> body ++;
                    char *arg1 = malloc (sizeof(*arg1) * 256);
                    strcpy(arg1, compile (Ast -> ast_function -> body).name);
                    Ast -> ast_function -> body --;
                    if (Ast -> ast_function -> function[0] == '*')
                    {
                        fprintf(outfile, "\tmul\t%s\n", arg1);
                    }
                    else
                    {
                        fprintf(outfile, "\tmov\trdx, 0\n\tdiv\t%s\n", arg1);
                    }
                    outreg.type = 0;
                    strcpy (outreg.name, "rax");
                    free_register();
                    free(arg1);
                    return outreg;
                    break;
                case '!' :
                case '=' :
                case '>' :
                case '<' :
                {
                    char *cmp = malloc (sizeof(*cmp) * 3);
                    if ((Ast -> ast_function -> function[0] == '<') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "le");
                    else if (Ast -> ast_function -> function[0] == '<') strcpy (cmp, "l");
                    else if ((Ast -> ast_function -> function[0] == '>') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "ge");
                    else if (Ast -> ast_function -> function[0] == '>') strcpy (cmp, "g");
                    else if ((Ast -> ast_function -> function[0] == '=') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "e");
                    else if ((Ast -> ast_function -> function[0] == '!') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "ne");
                    else if (Ast -> ast_function -> function[0] == '=')
                    {
                        int index = varindex (Ast -> ast_function -> body -> ast_identifier -> name);
                        Ast -> ast_function -> body ++;
                        struct reg arg = compile (Ast -> ast_function -> body);
                        char *arg2 = malloc (sizeof(*arg1) * 256);
                        strcpy (arg2, arg.name);
                        if ((symbol_table + index) -> type == -1)
                        {
                            (symbol_table + index) -> type = arg.type;
                            (symbol_table + index) -> is_static = 0;
                        }
                        fprintf (outfile, "\tmov\t[%s], %s\n", (symbol_table + index) -> name, arg2);
                        free(arg2);
                        free(cmp);
                        free(arg.name);
                        free_register();
                        Ast -> ast_function -> body --;
                        break;
                    }
                    char *arg1 = compile (Ast -> ast_function -> body).name;
                    Ast -> ast_function -> body ++;
                    char *arg2 = compile (Ast -> ast_function -> body).name;
                    used_functions[1] = 1;
                    Ast -> ast_function -> body --;
                    fprintf(outfile,"\tcmp\t%s, %s\n\tj%s\t_true%d\n\tmov\trax, 0\n\tjmp\t_after%d\n_true%d:\n\tmov\trax, 1\n_after%d:\n",
                            arg1,
                            arg2,
                            cmp,
                            number_cmp,
                            number_cmp,
                            number_cmp,
                            number_cmp);
                    free_register();
                    free_register();
                    outreg.type = 2;
                    strcpy (outreg.name, "rax");
                    free(arg1);
                    free(arg2);
                    free (cmp);
                    return outreg;
                }
                default :
                    if (!strcmp("print", Ast -> ast_function -> function))
                    {
                        struct reg arg = compile(Ast -> ast_function -> body);
                        if (arg.type == 0)
                        {
                            fprintf(outfile, "\tmov\trsi, %s\n\tmov\trdi, int_to_str\n\txor rax, rax\n\tcall\tprintf wrt ..plt\n\txor\trax, rax\n", arg.name);
                            free_register();
                        }
                        else if (arg.type == 1)
                        {
                            fprintf(outfile, "\tmov\teax,4\n\tmov\tebx,1\n\tmov\tecx,%s\n\tmov\tedx,%s_len\n\tint\t80h\n", arg.name, arg.name);
                        }
                        free(arg.name);
                    }
            }
            break;
        case 2 :
        {
            int reg = new_register();
            fprintf(outfile, "\tmov\t%s, %d\n", reglist[reg], Ast -> ast_number -> value);
            strcpy (outreg.name, reglist[reg]);
            outreg.type = 0;
            return outreg;
        }
        case 3 :
            break;
        case 4 :
            (symbol_table + varind) -> is_static = 1;
            char *str_name = malloc (sizeof(*str_name) * 256);
            sprintf(str_name, "str%d", number_stings);
            number_stings ++;
            strcpy((symbol_table + varind) -> name, str_name);
            strcpy((symbol_table + varind) -> string, Ast -> ast_string -> value);
            (symbol_table + varind) -> type = 1;
            varind ++;
            outreg.type = 1;
            strcpy (outreg.name, str_name);
            free(str_name);
            return outreg;
        case 5 :
            strcpy((symbol_table + varind) -> name, Ast -> ast_vardeclaration -> name);
            (symbol_table + varind) -> type = -1;
            varind ++;
            break;
        case 6 :
        {
            struct reg condition = compile (Ast -> ast_if -> condition);
            fprintf(outfile, "\tcmp\t%s, 0\n", condition.name);
            free(condition.name);
            if ((Ast + 1) -> type == 9)
            {
                fprintf(outfile, "\tje\t_else%d\n", nubmer_structures);
            }
            else
            {
                fprintf(outfile, "\tje\t_aft%d\n", nubmer_structures);
            }
            for (int i = 0; i < Ast -> ast_if -> body_length; i ++)
            {
                struct reg comp = compile (Ast -> ast_if -> body);
                if (comp.type != -1)
                {
                    outreg.type = comp.type;
                    strcpy (outreg.name, comp.name);
                }
                Ast -> ast_if -> body ++;
                free(comp.name);
            }
            Ast -> ast_if -> body -= Ast -> ast_if -> body_length;
            if ((Ast + 1) -> type == 9)
            {
                struct reg tmpreg;
                tmpreg.name = malloc (sizeof(*tmpreg.name) * 256);
                fprintf(outfile, "\tjmp\t_aft%d\n_else%d:\n", nubmer_structures, nubmer_structures);
                Ast ++;
                for (int i = 0; i < Ast -> ast_else -> body_length; i ++)
                {
                    struct reg comp = compile (Ast -> ast_else -> body);
                    if (comp.type != -1)
                    {
                        tmpreg.type = comp.type;
                        strcpy (tmpreg.name, comp.name);
                    }
                    Ast -> ast_else -> body ++;
                    free (comp.name);
                }
                Ast -> ast_else -> body -= Ast -> ast_else -> body_length;
                Ast --;
                if (outreg.type != -1)
                {
                    puts("aa");
                    fprintf(outfile, "\tmov\t%s, %s\n", outreg.name, tmpreg.name);
                }
                free(tmpreg.name);
            }
            fprintf(outfile, "_aft%d:\n",nubmer_structures);
            nubmer_structures ++;
            return outreg;
        }
        case 7 :
        {
            fprintf(outfile, "_start%d:\n",  nubmer_structures);
            char *condition = malloc (sizeof(*condition) * 256);
            strcpy (condition, compile (Ast -> ast_if -> condition).name);
            fprintf(outfile, "\tcmp\t%s, 0\n\tje\t_aft%d\n", condition, nubmer_structures);
            for (int i = 0; i < Ast -> ast_while -> body_length; i ++)
            {
                compile (Ast -> ast_while -> body);
                Ast -> ast_while -> body ++;
            }
            Ast -> ast_while -> body -= Ast -> ast_while -> body_length;
            fprintf(outfile, "\tjmp\t_start%d\n_aft%d:\n", nubmer_structures, nubmer_structures);
            nubmer_structures ++;
            break;
        }
        case 8 :
        {
            if ((symbol_table + varindex (Ast -> ast_identifier -> name)) -> type == -1)
            {
                puts("Error : used of non initialized variable.");
                exit(1);
            }
            int reg = new_register();
            fprintf(outfile, "\tmov\t%s, [%s]\n", reglist[reg], Ast -> ast_identifier -> name);
            strcpy (outreg.name, reglist[reg]);
            outreg.type = (symbol_table + varindex(Ast -> ast_identifier -> name)) -> type;
            return outreg;
        }
        case 9 :
            break;
        case 10 :
            break;
        case 11 :
        {
            int ast_length =  Ast -> ast -> length;
            for (int i = 0; i < ast_length; i ++)
                {
                    struct reg comp = compile (Ast -> ast);
                    if (comp.type != -1)
                    {
                        outreg.type = comp.type;
                        strcpy (outreg.name, comp.name);
                    }
                    Ast -> ast ++;
                }
            Ast -> ast -= ast_length;
            return outreg;
            break;
        }
    }
    return outreg;
}

void epilog()
{
    fprintf(outfile, "\tmov\teax,1\n\tmov\tebx,0\n\tint\t80h\n");
    for (int i = 0; i < varind ; i ++)
    {
        if ((symbol_table + i) -> type == 3)
        {
            fprintf(outfile, "_%s:\n", (symbol_table + i) -> name);
            compile((symbol_table + i) -> ast);
        }
    }
    fprintf(outfile, "section .data\n");
    for (int i = 0; i < varind ; i ++)
    {
        if ((symbol_table + i) -> is_static == 1)
        {
            fprintf(outfile, "\t%s:\t db '%s',10\n\t%s_len:\tequ $-%s\n",
                    (symbol_table + i) -> name,
                    (symbol_table + i) -> string,
                    (symbol_table + i) -> name,
                    (symbol_table + i) -> name);
        }
    }
    fprintf(outfile, "\tint_to_str:\t db '%%d',0xA\nsection .bss\n");
    for (int i = 0; i < varind ; i ++)
    {
        if ((symbol_table + i) -> is_static == 0)
        {
            fprintf(outfile, "\t%s:\t resb 8\n",
                    (symbol_table + i) -> name);
        }
    }
}

int main ( int argc, char *argv[] ){
    if (argc != 2)
    {
        exit(1);
    }
    fp1 = fopen (argv[1], "r");
    fseek(fp1, 0L, SEEK_END);
    file_length = ftell(fp1);
    rewind(fp1);
    outfile = fopen ("out.asm", "w");
    fprintf(outfile, "section\t.text\nglobal\tmain\nextern\tprintf\nmain:\n");
    struct parse outfinal;
    symbol_table  = malloc(20 * sizeof(struct variable));
    outfinal.size = 0;
    outfinal.body = malloc(10 * sizeof(struct leaf));
    while(1)
    {
        struct token *tokens;
        tokens = malloc(25 * sizeof(struct token));
        struct parse out = parsestatement(lexer(fp1, 0, tokens), "\n");
        for (int i = 0; i < out.size; i ++){
            copy_ast(out.body, outfinal.body, 0, outfinal.size);
            outfinal.size ++;
            freeall(out.body);
            out.body ++;
        }

        free(tokens);
        if (out.size == -1){
            break;
        }
        out.body -= out.size;
        free(out.body);
    }
    fclose(fp1);
    for (int i = 0; i < outfinal.size; i++){
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    for (int i = 0; i < outfinal.size; i++){
        check(outfinal.body);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    for (int i = 0; i < outfinal.size; i++){
        free(compile(outfinal.body).name);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    for (int i = 0; i < outfinal.size; i++){
        freeall(outfinal.body);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    epilog();
    free(symbol_table);
    free(outfinal.body);
    fclose(outfile);
    system("nasm -f elf64 ./out.asm");
    system("gcc ./out.o -o out -no-pie");
    exit(0);
}
