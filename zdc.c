/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2019-2020, Aghilas Boussaa
 * For more details see LICENSE
 *
 * Here is the Zdeta compiler A.K.A "zdc"
 * By @Yul3n
 * Usage zdc "program to compile" (optionnal : -o "name of the output file" or -lib to generate a library file)
 */

// Import the libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"

/* The constants used by the compiler */
#define SYMBOL_TABLE_SIZE 100

// Define an enumeration to list all the possible token types
enum type { OPERATOR,
            SEPARATOR,
            IDENTIFIER,
            NUMBER,
            KEYWORD,
            STRING };
// Define an enumeration to list all the possible instruction types
enum instruction_type{ FUNCTION,
                       FUNCTION_CALL,
                       ZNUMBER,
                       ZSTRING,
                       VARDECLARATION,
                       IFSTATEMENT,
                       WHILESTATEMENT,
                       ZIDENTIFIER,
                       ELSESTATEMENT,
                       ELIFSTATEMENT,
                       AST,
                       REG,
                       STACK_POS};
enum variable_type{ INTEGER,
                    CHAR_LIST,
                    BOOL,
                    FUNCTION_DECL};
// Define the structure for a token with a type and a name
struct token
{
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
        struct elsestatement        *ast_else;
        struct elifstatement        *ast_elif;
        struct reg                  *ast_register;
        int                          stack_pos;
    };
    int length; // Useful for the arrays
    int is_negative;
};
// Define the structures for the different AST node types
struct function {
    char         name[50];
    int          body_length;
    int          argnumber;
    char         arguments[5][10];
    struct leaf *body;
};
struct whilestatement {
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct ifstatement {
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct elifstatement {
    int          body_length;
    struct leaf *condition;
    struct leaf *body;
};
struct elsestatement {
    int          body_length;
    struct leaf *body;
};
struct functioncall{
    int          body_length;
    char         function[15];
    struct leaf *body;
};
struct number {
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
    int          size;
    int          used_tokens;
    int          used_structures;
};
struct variable{
    char               name[20];
    enum variable_type type;
    enum variable_type func_type;
    int                is_static;
    int                array_length;
    int                At_the_end_0xA;
    union
    {
        char        string[100];
        int         integer;
        struct leaf *ast;
    };
};
struct reg { // the structure of a register and its name
    char              *name;
    enum variable_type type;
};
char opps[11] = {'>','<', '=', '!', '+', '/', '-', '%','^', '*','<'}; // List of all operators
char symbols[11] = {'(',')','{','}','"','[',']',',','#','\n', ':'}; // List of all symbols
char *keywords[12] = {"let", "print", "while", "if", "else",
                  "elif", "string", "match", "iter", "read", "int", "include"}; // List of keybords
FILE *fp1, *outfile;
struct variable *symbol_table;
int varind           = 0, // Keep track of the actual free symbol table place
    indentlevel      = 0,
    number_functions = 0;
char custom_functions[10][10];
int arg_number[10];

/*
 * The following functions are utils used by the compiler itself.
 *
 */

static void
error(char error[])
{
    printf("\033[1;31mError\033[0m : %s\n", error);
    exit(1);
}

static void
warning(char warning[])
{
    printf("\033[1;33mError\033[0m : %s\n", warning);
}

/* A function to check if a string is a keyword */
int iskeyword (char in[]){
    for (int i = 0; i < 11; i++) if (strcmp(keywords[i], in) == 0) return 1;
    return 0;
}
// A function to check if a char is in a char list
int
isinchars(char in[], char check)
{
    for (int i = 0; i < 11; i++) if (in[i] == check) return 1;
    return 0;
}

/* This function checks if a string is in a given list of strings */
int
is_in_strings(char in[], char list[][10], int length)
{
    for (int i = 0; i < length; i++) if (strcmp(list[i], in) == 0) return 1 + i;
    return 0;
}

/* A function to check the operator precedence of an input string */
int
operatorPrecedence (char operator[])
{
    int precedence = -1; // If the input is not an operator or is empty, return -1
    if (((strcmp(operator,   "&")) == 0) ||
        ((strcmp(operator,   "|")) == 0) ||
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

/*
 * Takes the name of a variable and return its position in the symbol table or
 * throws an error if it doesn't exist.
 */
int
varindex (char var[])
{
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
        if (strcmp((symbol_table + i) -> name, var) == 0)
            return i;
    error("Using non declarated variable.");
}

void freeall(struct leaf *Ast){
    switch(Ast -> type){
        case FUNCTION:
            /*	FALLTHROUGH */
        case FUNCTION_CALL:
            /*	FALLTHROUGH */
        case IFSTATEMENT:
            /*	FALLTHROUGH */
        case WHILESTATEMENT:
            /*	FALLTHROUGH */
        case ELSESTATEMENT:
            /*	FALLTHROUGH */
        case ELIFSTATEMENT: {
        int body_length = 0;
            int has_condition = 1;
            struct leaf *body;
            struct leaf *condition;
            if (Ast -> type == IFSTATEMENT) {
                condition = Ast -> ast_if -> condition;
                body_length = Ast -> ast_if -> body_length;
                body = Ast -> ast_if -> body;
            } else if (Ast -> type == WHILESTATEMENT) {
                condition = Ast -> ast_while -> condition;
                body_length = Ast -> ast_while -> body_length;
                body = Ast -> ast_while -> body;
            } else if (Ast -> type == ELSESTATEMENT) {
                has_condition = 0;
                body_length = Ast -> ast_else -> body_length;
                body = Ast -> ast_else -> body;
            } else if (Ast -> type == ELIFSTATEMENT){
                condition = Ast -> ast_elif -> condition;
                body_length = Ast -> ast_elif -> body_length;
                body = Ast -> ast_elif -> body;
            } else if (Ast -> type == FUNCTION_CALL){
                has_condition = 0;
                body_length = Ast -> ast_function -> body_length;
                body = Ast -> ast_function -> body;
            } else {
                has_condition = 0;
                body_length = Ast -> ast_functiondeclaration -> body_length;
                body = Ast -> ast_functiondeclaration -> body;
            }
            for (int i = 0; i < body_length; i++){
                freeall(body + i);
            }
            free(body);
            if (has_condition) {
                freeall(condition);
                free(condition);
            }
            if      (Ast -> type == IFSTATEMENT)    free(Ast -> ast_if);
            else if (Ast -> type == ELSESTATEMENT)  free(Ast -> ast_else);
            else if (Ast -> type == ELIFSTATEMENT)  free(Ast -> ast_elif);
            else if (Ast -> type == WHILESTATEMENT) free(Ast -> ast_while);
            else if (Ast -> type == FUNCTION_CALL)  free(Ast -> ast_function);
            else                                    free(Ast -> ast_functiondeclaration);
            break;
        }
        case ZNUMBER:
            free(Ast -> ast_number);
            break;
        case ZSTRING:
            free(Ast -> ast_string);
            break;
        case VARDECLARATION:
            free(Ast -> ast_vardeclaration);
            break;
        case ZIDENTIFIER:
            if (Ast -> ast_identifier -> has_index) {
                freeall(Ast -> ast_identifier -> index);
                free(Ast -> ast_identifier -> index);
            }
            free(Ast -> ast_identifier);
            break;
        case AST:
            for (int i = 0; i < Ast -> ast -> length; i++){
                freeall(Ast -> ast);
                Ast -> ast ++;
            }
            break;
        case REG:
            free(Ast -> ast_register -> name);
            free(Ast -> ast_register);
            break;
        case STACK_POS: break;
    }
}

/*
 * A function to copy the content of an AST structure to another one
 */

void
copy_ast(struct leaf *transmitter, struct leaf *receiver, int index1, int index2)
{
    /* Adjust the pointers of each AST according to the input values */
    transmitter += index1;
    receiver += index2;
    switch(transmitter -> type){
        case FUNCTION:
            receiver -> ast_functiondeclaration = (struct function*) malloc(sizeof(struct function));
            receiver -> ast_functiondeclaration -> body = (struct leaf*) malloc((transmitter -> ast_functiondeclaration -> body_length) * sizeof(struct leaf));
            strcpy(receiver -> ast_functiondeclaration -> name, transmitter -> ast_functiondeclaration -> name);
            receiver -> ast_functiondeclaration -> argnumber = transmitter -> ast_functiondeclaration -> argnumber;
            receiver -> ast_functiondeclaration -> body_length = transmitter -> ast_functiondeclaration -> body_length;
            for (int i = 0; i < transmitter -> ast_functiondeclaration -> argnumber; i++)
                strcpy(receiver -> ast_functiondeclaration -> arguments[i], transmitter -> ast_functiondeclaration -> arguments[i]);
            for (int i = 0; i < (transmitter -> ast_functiondeclaration -> body_length); i++)
                copy_ast(transmitter -> ast_functiondeclaration -> body, receiver -> ast_functiondeclaration -> body, i, i);
            receiver -> length = 1;
            break;
        case FUNCTION_CALL:
            receiver -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
            strcpy(receiver -> ast_function -> function, transmitter -> ast_function -> function);
            receiver -> ast_function -> body = (struct leaf*) malloc((transmitter -> ast_function -> body_length) * sizeof(struct leaf));
            receiver -> ast_function -> body_length = transmitter -> ast_function -> body_length;
            for (int i = 0; i < (transmitter -> ast_function -> body_length); i++)
                copy_ast(transmitter -> ast_function -> body, receiver -> ast_function -> body, i, i);
            receiver -> length = 1;
            break;
        case ZNUMBER:
            receiver -> ast_number = (struct number*) malloc(transmitter -> length * sizeof(struct number));
            for (int i = 0; i < transmitter -> length; i++)
            {
                receiver -> ast_number -> value = transmitter -> ast_number -> value;
                receiver -> ast_number ++;
                transmitter -> ast_number ++;
            }
            receiver -> ast_number -= transmitter -> length;
            transmitter -> ast_number -= transmitter -> length;
            receiver -> length = transmitter -> length;
            break;
        case ZSTRING:
            receiver -> ast_string = (struct string*) malloc(sizeof(struct string));
            strcpy(receiver -> ast_string -> value, transmitter -> ast_string -> value);
            break;
        case VARDECLARATION:
            receiver -> ast_vardeclaration = (struct variable_declaration*) malloc(sizeof(struct variable_declaration));
            strcpy(receiver -> ast_vardeclaration -> name, transmitter -> ast_vardeclaration -> name);
            break;
        case IFSTATEMENT:
            receiver -> ast_if = (struct ifstatement*) malloc(sizeof(struct ifstatement));
            receiver -> ast_if -> body_length = transmitter -> ast_if -> body_length;
            receiver -> ast_if -> body = (struct leaf*) malloc((transmitter -> ast_if -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_if -> body_length); i++)
                copy_ast(transmitter -> ast_if -> body, receiver -> ast_if -> body, i, i);
            receiver -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_if -> condition, receiver -> ast_if -> condition, 0, 0);
            break;
        case WHILESTATEMENT:
            receiver -> ast_while = (struct whilestatement*) malloc(sizeof(struct whilestatement));
            receiver -> ast_while -> body_length = transmitter -> ast_while -> body_length;
            receiver -> ast_while -> body = (struct leaf*) malloc((transmitter -> ast_while -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_while -> body_length); i++)
                copy_ast(transmitter -> ast_while -> body, receiver -> ast_while -> body, i, i);
            receiver -> ast_while -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_while -> condition, receiver -> ast_while -> condition, 0, 0);
            break;
        case ZIDENTIFIER:
            receiver -> ast_identifier = (struct identifier*) malloc(sizeof(struct identifier));
            receiver -> ast_identifier -> has_index = transmitter -> ast_identifier -> has_index;
            if (1 == transmitter -> ast_identifier -> has_index)
            {
                receiver -> ast_identifier -> index = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(transmitter -> ast_identifier -> index, receiver -> ast_identifier -> index, 0, 0);
            }
            strcpy (receiver -> ast_identifier -> name, transmitter -> ast_identifier -> name);
            receiver -> length = 1;
            break;
        case ELSESTATEMENT:
            receiver -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
            receiver -> ast_else -> body_length = transmitter -> ast_else -> body_length;
            receiver -> ast_else -> body = (struct leaf*) malloc((transmitter -> ast_else -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_else -> body_length); i++){
                copy_ast(transmitter -> ast_else -> body, receiver -> ast_else -> body, i, i);
            }
            break;
        case ELIFSTATEMENT:
            receiver -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
            receiver -> ast_elif -> body_length = transmitter -> ast_elif -> body_length;
            receiver -> ast_elif -> body = (struct leaf*) malloc((transmitter -> ast_elif -> body_length) * sizeof(struct leaf));
            for (int i = 0; i < (transmitter -> ast_elif -> body_length); i++)
                copy_ast(transmitter -> ast_elif -> body, receiver -> ast_elif -> body, i, i);
            receiver -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(transmitter -> ast_elif -> condition, receiver -> ast_while -> condition, 0, 0);
            break;
        case AST:
        {
            int j = transmitter -> ast -> length;
            receiver -> ast = (struct leaf*) malloc(j * sizeof(struct leaf));
            receiver -> ast -> length = j;
            for (int i = 0; i < j; i++)
                copy_ast(transmitter -> ast, receiver -> ast, i, i);
            break;
        }
        case REG:
            receiver -> ast_register = (struct reg*) malloc(sizeof(struct reg));
            receiver -> ast_register -> name = malloc(256 * sizeof(*receiver -> ast_register -> name));
            strcpy(receiver -> ast_register -> name, transmitter -> ast_register -> name);
            break;
        case STACK_POS:
            receiver -> stack_pos = transmitter -> stack_pos;
    }
    receiver -> is_negative = transmitter -> is_negative;
    receiver -> type = transmitter -> type;
}

void
move_ast(struct leaf *transmitter, struct leaf *receiver, int index1, int index2)
{

    copy_ast(transmitter, receiver, index1, index2);
    freeall(transmitter + index1);
}

/*
 * The Lexer
 * Takes the input file as an input and return a list of tokens corresponding to a vlock of code.
 * For instance :
 * print 4 -> [keyword: 'print'], [number: '4'], [separator: 'switch_indent']
 */
struct lexline
lexer(FILE *fp1, int min_indent, struct token *tokens)
{
    struct lexline lex;
    char c = ' ';
    int i = 0, j = 0, k = 0, l = 0;
    char buffer[60];
    char conv[2] = {'a', '\0'};
    while (c != EOF){
        c = fgetc(fp1);
        char d = ' ';
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
            if (iskeyword(buffer)) (tokens+k) -> type = KEYWORD;
            else (tokens + k) -> type = IDENTIFIER;
            strcpy((tokens + k) -> value, buffer);
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
            (tokens + k) -> type = NUMBER;
            strcpy((tokens + k) -> value, buffer);
            k++;
        }
        else if (isinchars(opps, c)){
            if (k >= 1)
                if (((tokens + k - 1) -> type == 0) && ((tokens + k - 1) -> value [0] != '=')) error("Unexpected combination of opperators");
            (tokens + k) -> type = 0;
            d = fgetc(fp1);
            if ((c == '=') && (d == '=')) strcpy((tokens + k) -> value, "==");
            else if ((c == '<') && (d == '=')) strcpy((tokens + k) -> value, "<=");
            else if ((c == '>') && (d == '=')) strcpy((tokens + k) -> value, ">=");
            else if ((c == '=') && (d == '>')) strcpy((tokens + k) -> value, "=>");
            else if ((c == '!') && (d == '=')) strcpy((tokens + k) -> value, "!=");
            else if ((c == '/') && (d == '/')){
                while (c != '\n') c = fgetc(fp1);
                k --;
            }
            else{
                conv[0] = c;
                strcpy((tokens + k) -> value, conv);
                fseek(fp1, ftell(fp1) - 1, SEEK_SET);
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
                while (c != '#') c = fgetc(fp1);
                k --;
                c = fgetc(fp1);
            }
            else if (c == ':')
            {
                c = fgetc(fp1);
                if (c == ':')
                {
                    (tokens + k) -> type = 1;
                    strcpy((tokens + k) -> value, "::");
                }
                else
                {
                    fseek(fp1, ftell(fp1) - 1, SEEK_SET);
                    (tokens + k) -> type = 1;
                    conv[0] = ':';
                    strcpy((tokens + k) -> value, conv);
                }
            }
            else {
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
                    if (c != EOF) fseek(fp1, ftell(fp1) - 1, SEEK_SET);
                    indent /= 2;
                    if (indent < indentlevel)
                    {
                        k ++;
                        indentlevel = indent;
                        strcpy((tokens + k) -> value, "switch_indent");
                    }
                    if (indent <= min_indent){
                        indentlevel = indent;
                        strcpy((tokens + k) -> value, "switch_indent");
                        lex.size = k;
                        lex.base_value = 0;
                        lex.tokens = tokens;
                        return lex;
                    }
                    indentlevel = indent;
                }
            }
            k++;
        }
    }
    lex.size = -1;
    return lex;
}

/*
 * The parser
 * Takes a list of tokens from the lexer and outputs a list of ASTs.
 * For instance :
 * [keyword: 'print'], [number: '4'], [separator: 'switch_indent'] ->
 * function_call : print
 *     argument 1 :
 *         number : 4
 */
struct parse parsestatement(struct lexline lex, char terminator2[20], int max_length)
{
    if (lex.size == -1)
    {
        struct parse output;
        output.size = -1;
        return(output);
    }

    struct leaf *arg2;
    struct leaf *Ast;
    struct token *tokens = lex.tokens;
    struct token operators[5];
    int aindex = 0;
    int size = lex.base_value;
    int current_operator = 0;
    int used_structures = 0;

    for (int i = 0; i < 5; i++) strcpy(operators[i].value, " ");
    Ast = (struct leaf*) malloc(30 * sizeof(struct leaf));
    while (size <= lex.size){
        if (size > 0)
        {
            if (((tokens + size - 1) -> value[0] == '-')
                && ((((tokens + size - 2) -> type == 0))
                    || ((tokens + size - 2) -> type == 1)
                    || (is_in_strings((tokens + size - 2) -> value, custom_functions, number_functions))))
            {
                (Ast + aindex) -> is_negative = 1;
                current_operator --;
            }
            else (Ast + aindex) -> is_negative = 0;
        }
        if ((max_length != -1) && (max_length <= aindex)) break;
        struct token token;
        token.type = (tokens + size) -> type;
        strcpy(token.value, (tokens + size) -> value);
        if ((strcmp(token.value, terminator2) == 0)){
            size ++;
            break;
        }
        if (token.type == 3){
            (Ast + aindex) -> length = 1;
            (Ast + aindex) -> ast_number = (struct number*) malloc(sizeof(struct number));
            (Ast + aindex) -> type = ZNUMBER;
            (Ast + aindex) -> ast_number -> value = atoi(token.value);
            aindex ++;
            size ++;
        }
        else if (token.type == 5){
            (Ast + aindex) -> ast_string = (struct string*) malloc(sizeof(struct string));
            (Ast + aindex) -> type = ZSTRING;
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
                    (Ast + aindex - 2) -> type = FUNCTION_CALL;
                    strcpy(((Ast + aindex - 2) -> ast_function) -> function, operators[current_operator - 1].value);
                    copy_ast(arg2, (Ast + aindex - 2) -> ast_function -> body, 0, 0);
                    copy_ast(Ast, (Ast + aindex - 2) -> ast_function -> body, aindex - 1, 1);
                    ((Ast + aindex - 2) -> ast_function ) -> body_length = 2;
                    aindex --;
                    current_operator --;
                }
                else break;
            }
            strcpy(operators[current_operator].value, token.value);
            current_operator ++;
            size ++;
        }
        else if (token.type == 1)
        {
            struct parse argbody;
            switch (token.value[0]) {
                case '(':
                    size ++;
                    lex.base_value = size;
                    argbody = parsestatement(lex, ")", -1);
                    for (int i = 0; i < argbody.size; i ++){
                        move_ast(argbody.body, Ast + aindex, i, i);
                        aindex ++;
                    }
                    while ((tokens + size) -> value[0] != ')') size ++;
                    free(argbody.body);
                    size ++;
                    break;
                case '\n':
                    current_operator --;
                    while (current_operator >= 0) {
                        arg2 = (struct leaf *) malloc(sizeof(struct leaf));
                        copy_ast(Ast, arg2, aindex - 2, 0);
                        (Ast + aindex - 2) -> ast_function = (struct functioncall *) malloc(sizeof(struct functioncall));
                        (Ast + aindex - 2) -> ast_function->body = (struct leaf *) malloc(2 * sizeof(struct leaf));
                        (Ast + aindex - 2) -> type = FUNCTION_CALL;
                        strcpy(((Ast + aindex - 2) -> ast_function) -> function, operators[current_operator].value);
                        copy_ast(arg2, (Ast + aindex - 2)->ast_function->body, 0, 0);
                        copy_ast(Ast, (Ast + aindex - 2)->ast_function->body, aindex - 1, 1);
                        ((Ast + aindex - 2) -> ast_function)->body_length = 2;
                        aindex --;
                        current_operator--;
                    }
                    current_operator = 0;
                    size ++;
                    break;
                case '{':
                    while(((tokens + size) -> value)[0] != '\n' ) size ++;
                    lex.base_value = size + 1;
                    argbody = parsestatement(lex, "}", -1);
                    (Ast + aindex) -> type = AST;
                    (Ast + aindex) -> ast = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                    for (int i = 0; i < argbody.size; i ++)
                        move_ast(argbody.body, (Ast + aindex) -> ast, i, i);
                    free(argbody.body);
                    (Ast + aindex) -> length = argbody.size;
                    aindex ++;
                    while ((strcmp("}", (tokens + size) -> value)) && (size <= lex.size)) size ++;
                    break;
                case '}':
                case ')':
                    size ++;
                    break;
                case '[':
                    size ++;
                    int arraysize = 0;
                    while ((tokens + size) -> value[0] != ']') arraysize ++ ,size ++;
                    size -= arraysize;
                    (Ast + aindex) -> type = ZNUMBER;
                    (Ast + aindex) -> ast_number = (struct number*) malloc(arraysize * sizeof(struct number));
                    (Ast + aindex) -> length = arraysize;
                    while ((tokens + size) -> value[0] != ']') {
                        if ((tokens + size) -> type == 3) {
                            (Ast + aindex) -> ast_number -> value = atoi((tokens + size) -> value);
                            (Ast + aindex) -> ast_number ++;
                        }
                        size ++;
                    }
                    (Ast + aindex) -> ast_number -= arraysize;
                    size ++;
                    aindex ++;
                default:
                    size ++;
                    break;
            }
        } else if (token.type == 4){
            if (strcmp(token.value, "let") == 0){
                if ((tokens + size + 1) -> type != 2)
                    error("Can't assign to something that is not an identifier.");
                int is_function = 0;
                int size_before = size;
                while (((tokens + size) -> value[0] != '\n') && (strcmp((tokens + size) -> value, "switch_indent"))) { // We first search if we have a "=>" token on the line
                    if (!strcmp((tokens + size) -> value, "=>")) {
                        is_function = size; // If we encouter the "=>" token it means that we're declaring a function, so we store its position
                        break;
                    }
                    size ++;
                }
                size = size_before; // Set back the position to the one we were before
                if (is_function == 0) // If we haven't encountered a => it is a variable declaration
                {
                    (Ast + aindex) -> type = VARDECLARATION;
                    (Ast + aindex) -> ast_vardeclaration = (struct variable_declaration *) malloc(sizeof(struct variable_declaration));
                    strcpy((Ast + aindex) -> ast_vardeclaration -> name, (tokens + size + 1) -> value); // Name the variable after the next token
                } else {
                    (Ast + aindex) -> type = FUNCTION;
                    size ++;
                    (Ast + aindex) -> ast_functiondeclaration = (struct function*) malloc(sizeof(struct function));
                    strcpy((Ast + aindex) -> ast_functiondeclaration -> name, (tokens + size) -> value); // Name the function after the token before let
                    (Ast + aindex) -> ast_functiondeclaration -> argnumber = 0;
                    size ++;
                    while (size < is_function) // Use all the tokens between the function name and the => as arguments
                    {
                        strcpy((Ast + aindex) -> ast_functiondeclaration -> arguments[(Ast + aindex) -> ast_functiondeclaration -> argnumber],
                               (tokens + size) -> value);
                        (Ast + aindex) -> ast_functiondeclaration -> argnumber ++;
                        size ++;
                    }
                    arg_number[number_functions] = (Ast + aindex) -> ast_functiondeclaration -> argnumber; // Specify the number of argument of the function
                    lex.base_value = is_function + 2;
                    strcpy (custom_functions[number_functions], (Ast + aindex) -> ast_functiondeclaration -> name); // Add the name of the function to the list of custom functions
                    number_functions ++;
                    struct parse argbody = parsestatement(lex, "switch_indent", -1);
                    (Ast + aindex) -> ast_functiondeclaration -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                    for (int i = 0; i < argbody.size; i ++) // Copy all ASTs to the function body
                    {
                        copy_ast(argbody.body, (Ast + aindex) -> ast_functiondeclaration -> body, 0, 0);
                        (Ast + aindex) -> ast_functiondeclaration -> body ++;
                        freeall(argbody.body);
                        argbody.body ++;
                    }
                    (Ast + aindex) -> ast_functiondeclaration -> body_length = argbody.size;
                    argbody.body -= argbody.size;
                    free(argbody.body);
                    (Ast + aindex) -> ast_functiondeclaration -> body -= argbody.size;
                    int i = 0;
                    used_structures += argbody.used_structures;
                    while (i <= argbody.used_structures)
                    {
                        if (!strcmp("switch_indent", (tokens + size) -> value)) i ++;
                        size ++;
                    } // Move at the end the block
                    size --;
                }
            }
            else if (strcmp(token.value, "if") == 0){
                (Ast + aindex) -> ast_if = (struct ifstatement*) malloc(sizeof(struct ifstatement)); // First allocate memory for the if statement
                (Ast + aindex) -> type = IFSTATEMENT;
                lex.base_value = size + 1; // Remove the if token
                struct parse argcondition = parsestatement(lex, "\n", -1); // First parses the condition
                while(((tokens + size) -> value)[0] != '\n') size ++; // remove tokens until we reached \n
                lex.base_value = size + 1; // Remove the \n token
                struct parse argbody = parsestatement(lex, "switch_indent", -1); // Now parses the body
                (Ast + aindex) -> ast_if -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf)); // Allocate memory for the condition and the body
                (Ast + aindex) -> ast_if -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_if -> condition, 0, 0); // Copy the condition to the AST
                for (int i = 0; i < argbody.size; i ++) // Now add the body to the AST
                    move_ast(argbody.body, (Ast + aindex) -> ast_if -> body, i, i);
                (Ast + aindex) -> ast_if -> body_length = argbody.size;
                int i = 0;
                used_structures = argbody.used_structures;
                while (i <= argbody.used_structures)
                {
                    if (!strcmp("switch_indent", (tokens + size) -> value)) i ++;
                    size ++;
                } // Move until the end of the indent block
                size --;
                free(argbody.body); // Free the memory we don't need
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else if (strcmp(token.value, "while") == 0)
            {
                (Ast + aindex) -> ast_while = (struct whilestatement*) malloc(sizeof(struct whilestatement));
                size ++;
                lex.base_value = size;
                struct parse argcondition = parsestatement(lex, "\n", -1);
                while(((tokens + size) -> value)[0] != '\n' ) size ++;
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent", -1);
                (Ast + aindex) -> type = WHILESTATEMENT;
                (Ast + aindex) -> ast_while -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                (Ast + aindex) -> ast_while -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_while -> condition, 0, 0);
                for (int i = 0; i < argbody.size; i ++)
                    move_ast(argbody.body, (Ast + aindex) -> ast_while -> body, i, i);
                (Ast + aindex) -> ast_while -> body_length = argbody.size;
                free(argbody.body);
                int i = 0;
                used_structures = argbody.used_structures;
                while (i <= argbody.used_structures)
                {
                    if (!strcmp("switch_indent", (tokens + size) -> value)) i ++;
                    size ++;
                }
                size --;
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else if (strcmp(token.value, "else") == 0){
                (Ast + aindex) -> ast_else = (struct elsestatement*) malloc(sizeof(struct elsestatement));
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent", -1);
                (Ast + aindex) -> type = ELSESTATEMENT;
                (Ast + aindex) -> ast_else -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                for (int i = 0; i < argbody.size; i ++)
                    move_ast(argbody.body, (Ast + aindex) -> ast_else -> body, i, i);
                (Ast + aindex) -> ast_else -> body_length = argbody.size;
                free(argbody.body);
                int i = 0;
                used_structures = argbody.used_structures;
                do {
                    if (!strcmp("switch_indent", (tokens + size) -> value)) i ++;
                    size ++;
                } while (i <= argbody.used_structures);
                size --;
            }
            else if (strcmp(token.value, "elif") == 0){
                (Ast + aindex) -> ast_elif = (struct elifstatement*) malloc(sizeof(struct elifstatement));
                lex.base_value = size + 1;
                struct parse argcondition = parsestatement(lex, "\n", -1);
                while(((tokens + size) -> value)[0] != '\n' ){
                    size ++;
                }
                lex.base_value = size + 1;
                struct parse argbody = parsestatement(lex, "switch_indent", -1);
                (Ast + aindex) -> type = ELIFSTATEMENT;
                (Ast + aindex) -> ast_elif -> condition = (struct leaf*) malloc(sizeof(struct leaf));
                copy_ast(argcondition.body, (Ast + aindex) -> ast_elif -> condition, 0, 0);
                (Ast + aindex) -> ast_elif -> body = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                for (int i = 0; i < argbody.size; i ++)
                    move_ast(argbody.body, (Ast + aindex) -> ast_elif -> body, i, i);
                (Ast + aindex) -> ast_elif -> body_length = argbody.size;
                while (strcmp("switch_indent", (tokens + size) -> value)) size ++;
                freeall(argcondition.body);
                free(argcondition.body);
            }
            else {
                (Ast + aindex) -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                size ++;
                lex.base_value = size;
                struct parse argbody;
                (Ast + aindex) -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                (Ast + aindex) -> type = FUNCTION_CALL;
                strcpy((Ast + aindex) -> ast_function -> function, token.value);
                if ((!strcmp(token.value, "print")) || (!strcmp(token.value, "read"))) {
                    argbody = parsestatement (lex, "\n", -1);
                    if (argbody.size != 0) {
                        (Ast + aindex) -> ast_function -> body_length = 1;
                        /* If read is called with an argument, we print the argument and then call read */
                        if (!strcmp(token.value, "read")) {
                            (Ast + aindex) -> ast_function -> body -> ast_function = (struct functioncall*) malloc(sizeof(struct functioncall));
                            (Ast + aindex) -> ast_function -> body -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                            move_ast(argbody.body, ((Ast + aindex) -> ast_function -> body -> ast_function -> body), 0, 0);
                            (Ast + aindex) -> ast_function -> body -> type = FUNCTION_CALL;
                            (Ast + aindex) -> ast_function -> body -> ast_function -> body_length= 1;
                            strcpy((Ast + aindex) -> ast_function -> body -> ast_function -> function, "print");
                        } else {
                            (Ast + aindex) -> ast_function -> body_length = 1;
                            move_ast(argbody.body, (Ast + aindex) -> ast_function -> body, 0, 0);
                        }
                    } else if (strcmp(token.value, "read")) error("Using the print function without an argument");
                    /* If we call read without an argument, print nothing */
                    else (Ast + aindex) -> ast_function -> body_length = 0;
                    while ((((tokens + size) -> value)[0] != '\n') && (strcmp((tokens + size) -> value, "switch_indent"))) size ++;
                } else if (!strcmp(token.value, "int")) {
                    argbody = parsestatement (lex, "\n", 1);
                    if (argbody.size == 0) error("The function int needs an argument.");
                    strcpy((Ast + aindex) -> ast_function -> function, "int");
                    (Ast + aindex) -> ast_function -> body_length = 1;
                    (Ast + aindex) -> ast_function -> body = (struct leaf*) malloc(sizeof(struct leaf));
                    move_ast(argbody.body, (Ast + aindex) -> ast_function -> body, 0, 0);
                    size += argbody.used_tokens;
                }
                free(argbody.body);
                used_structures --;
            }
            aindex ++;
            used_structures ++;
            size ++;
        }
        else if (token.type == 2){
            int index_of_function = is_in_strings(token.value, custom_functions, number_functions);
            if (index_of_function)
            {
                lex.base_value = size + 1;
                struct parse argbody = parsestatement (lex, "", arg_number[index_of_function - 1]);
                (Ast + aindex) -> type = FUNCTION_CALL;
                (Ast + aindex) -> ast_function = (struct functioncall *) malloc(sizeof(struct functioncall));
                (Ast + aindex) -> ast_function -> body_length = arg_number[index_of_function - 1];
                strcpy((Ast + aindex) -> ast_function -> function, token.value);
                (Ast + aindex) -> ast_function -> body = (struct leaf *) malloc(arg_number[index_of_function - 1] * sizeof(struct leaf));
                if (argbody.size < arg_number[index_of_function - 1]) error("Not enough argument for a function");
                for(int i = 0; i < arg_number[index_of_function - 1]; i ++)
                    move_ast(argbody.body, (Ast + aindex) -> ast_function -> body, 0, 0);
                size += 1 + argbody.used_tokens;
                free(argbody.body);
            }
            else
            {
                (Ast + aindex) -> type = ZIDENTIFIER;
                (Ast + aindex) -> ast_identifier = (struct identifier*) malloc (sizeof(struct identifier));
                strcpy((Ast + aindex) -> ast_identifier -> name, token.value);
                size ++;
                if (!strcmp((tokens + size) -> value, "::"))
                {
                    lex.base_value = size + 1;
                    struct parse argbody = parsestatement (lex, "", 1);
                    (Ast + aindex) -> ast_identifier -> index = (struct leaf*) malloc(argbody.size * sizeof(struct leaf));
                    (Ast + aindex) -> ast_identifier -> has_index = 1;
                    move_ast(argbody.body, (Ast + aindex) -> ast_identifier -> index, 0, 0);
                    free(argbody.body);
                    size += 1 + argbody.used_tokens;
                }
                else (Ast + aindex) -> ast_identifier -> has_index = 0;
            }
            aindex ++;
        }
    }
    current_operator --;
    while (current_operator >= 0) {
        arg2 = (struct leaf *) malloc(sizeof(struct leaf));
        move_ast(Ast, arg2, aindex - 2, 0);
        (Ast + aindex - 2) -> ast_function = (struct functioncall *) malloc(sizeof(struct functioncall));
        (Ast + aindex - 2) -> ast_function -> body = (struct leaf *) malloc(2 * sizeof(struct leaf));
        (Ast + aindex - 2) -> type = FUNCTION_CALL;
        strcpy(((Ast + aindex - 2) -> ast_function) -> function,
               operators[current_operator].value);
        copy_ast(arg2, (Ast + aindex - 2) -> ast_function->body, 0, 0);
        copy_ast(Ast, (Ast + aindex - 2) -> ast_function->body, aindex - 1, 1);
        ((Ast + aindex - 2) -> ast_function) -> body_length = 2;
        (Ast + aindex - 2) -> length = 0;
        freeall(Ast + aindex - 1);
        (Ast + aindex - 2) -> is_negative = 0;
        freeall(arg2);
        free(arg2);
        aindex --;
        current_operator --;
    }
    struct parse output;
    output.body = Ast;
    output.size = aindex;
    output.used_tokens = size - lex.base_value;
    output.used_structures = used_structures;
    return(output);
}

void
check(struct leaf *Ast)
{
    if (Ast -> type == FUNCTION_CALL){
        if (strcmp(Ast -> ast_function -> function, "=") == 0)
            if (Ast -> ast_function -> body -> type != ZIDENTIFIER)
                error("assigning a value to a non variable element.");
    }
    else if (Ast -> type == ELSESTATEMENT) {
        if (((Ast - 1) -> type != IFSTATEMENT) && ((Ast - 1) -> type != ELIFSTATEMENT))
            error("using an else without an if.");
    }
    return;
}
void multi_replace(char identifiers[][10], struct leaf *Ast, int identifier_num, int ast_size);
// Functions for the actual compiler
void
replace_by_stack (char identifiers[][10], struct leaf *Ast, int identifier_num)
{
    switch (Ast -> type)
    {
        case FUNCTION       :
            multi_replace(identifiers, Ast -> ast_functiondeclaration -> body,
                          identifier_num, Ast -> ast_functiondeclaration -> body_length);
            break;
        case FUNCTION_CALL  :
            multi_replace(identifiers, Ast -> ast_function -> body, identifier_num,
                          Ast -> ast_function -> body_length);
            break;
        case IFSTATEMENT    :
            multi_replace(identifiers, Ast -> ast_if -> body, identifier_num,
                          Ast -> ast_if -> body_length);
            replace_by_stack(identifiers, Ast -> ast_if -> condition, identifier_num);
            break;
        case WHILESTATEMENT :
            multi_replace(identifiers, Ast -> ast_while -> body, identifier_num,
                          Ast -> ast_while -> body_length);
            replace_by_stack(identifiers, Ast -> ast_while -> condition,
                             identifier_num);
            break;
        case ELSESTATEMENT  :
            multi_replace(identifiers, Ast -> ast_else -> body, identifier_num,
                          Ast -> ast_else -> body_length);
        case ELIFSTATEMENT  :
            multi_replace(identifiers, Ast -> ast_elif -> body, identifier_num,
                          Ast -> ast_elif -> body_length);
            replace_by_stack(identifiers, Ast -> ast_elif -> condition, identifier_num);
        case AST            :
            multi_replace(identifiers, Ast -> ast, identifier_num,
                          Ast -> length );
        case REG            : break;
        case STACK_POS      : break;
        case ZNUMBER        : break;
        case ZSTRING        : break;
        case VARDECLARATION : break;
        case ZIDENTIFIER :
            if (is_in_strings(Ast -> ast_identifier -> name, identifiers, identifier_num))
            {
                int i = is_in_strings(Ast -> ast_identifier -> name, identifiers, identifier_num);
                Ast -> type = STACK_POS;
                Ast -> stack_pos = i - 1;
            }
            break;

    }
}

void
multi_replace(char identifiers[][10], struct leaf *Ast, int identifier_num, int ast_size)
{
    for (int i = 0; i < ast_size; i++)
        replace_by_stack(identifiers, Ast + i, identifier_num);
}
static char *reglist[8] = { "r8", "r9", "r10", "r11", "rcx", "rdx", "rdi", "rsi" }; // List of registers
int number_stings = 0, used_registers = 0, number_cmp = 0, nubmer_structures = 0, number_array = 0;
int used_functions[1] = {0};
static char arg_func_list[6][10] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

int
new_register (void)
{
    used_registers ++;
    return used_registers - 1;
}

void
free_register (void)
{
    if (used_registers > 0) used_registers --;
}

void
save_registers (void)
{
    fprintf(outfile, "\tpush\trax\n"); // Save registers on the stack
    for (int i = 0; i < 8; i++) fprintf(outfile, "\tpush\t%s\n", reglist[i]);
}

void
restore_registers (void)
{
    for (int i = 7; i >= 0; i--) fprintf(outfile, "\tpop\t%s\n", reglist[i]); // Restore the arguments from the stack
}

/*
 * The assembly generator.
 * Takes as an input a list of ASTs and writes corresponding unoptimized assembly
 * to the output file.
 * For instance:
 * function_call : print
 *     argument 1 :
 *         number : 4
 * ->
 * mov     r8, 4
 * push    rbp
 * mov     rsi, r8
 * mov     rdi, int_to_str
 * xor     rax, rax
 * call    printf wrt ..plt
 * xor     rax, rax
 * pop     rbp
 */
struct reg
compile (struct leaf *Ast)
{
    struct reg outreg;

    outreg.name = malloc (sizeof(*outreg.name) * 256);
    outreg.type = -1;
    switch (Ast -> type) {
        case FUNCTION :
            for (int i = 0; i < Ast -> ast_functiondeclaration -> body_length; i++)
            {
                replace_by_stack(Ast -> ast_functiondeclaration -> arguments,
                                                Ast -> ast_functiondeclaration -> body,
                                                Ast -> ast_functiondeclaration -> argnumber);
                Ast -> ast_functiondeclaration -> body ++;
            }
            Ast -> ast_functiondeclaration -> body -= Ast -> ast_functiondeclaration -> body_length;
            (symbol_table + varind) -> type = FUNCTION_DECL;
            (symbol_table + varind) -> ast = (struct leaf*) malloc(sizeof(struct leaf));
            copy_ast(Ast, (symbol_table + varind) -> ast, 0, 0);
            strcpy((symbol_table + varind) -> name, Ast -> ast_functiondeclaration -> name);
            varind ++;
            break;
        case FUNCTION_CALL :
            switch (Ast -> ast_function -> function[0])
            {
                case '-' :
                case '+' :
                {
                    int is_string_add = 0;
                    struct reg arg1 = compile(Ast -> ast_function -> body);
                    if ((Ast -> ast_function -> body -> type == ZSTRING) || (arg1.type == CHAR_LIST)) is_string_add = 1;
                    else if (Ast -> ast_function -> body -> type == ZIDENTIFIER)
                        if ((symbol_table + varindex(Ast -> ast_function -> body -> ast_identifier -> name)) -> type == CHAR_LIST) is_string_add = 1;
                    Ast -> ast_function -> body ++;
                    if ((Ast -> ast_function -> body -> type == ZNUMBER) && (is_string_add)) error("Adding a string and a number.");
                    struct reg arg2 = compile (Ast -> ast_function -> body);
                    Ast -> ast_function -> body --;
                    if (is_string_add) {
                        save_registers();
                        fprintf(outfile,
                                "\tmov\trdi, buffer\n"
                                "\tmov\trsi, hello\n"
                                "\tmov\trdx, 256\n"
                                "\tcall\tstrncpy\n");
                        restore_registers();
                        fprintf(outfile, "\tpop\trax\n"); // Restore registers on the stack
                        fprintf(outfile,
                                "\tmov\trdi,%s\n"
                                "\tmov\trsi,%s\n"
                                "\tcall\tstrcat\n"
                                "\tmov\trdi, out_buffer\n"
                                "\tmov\trsi, rax\n"
                                "\tmov\trdx, 256\n"
                                "\tcall\tstrncpy\n",
                                arg1.name,
                                arg2.name);
                        save_registers();
                        fprintf(outfile,
                                "\tmov\trdi, hello\n"
                                "\tmov\trsi, buffer\n"
                                "\tmov\trdx, 256\n"
                                "\tcall\tstrncpy\n");
                        restore_registers();
                        fprintf(outfile, "\tpop\trax\n"); // Restore registers on the stack
                        strcpy(outreg.name, "out_buffer");
                        outreg.type = CHAR_LIST;
                        for (int j = 0; j < 8; j++) if (!strcmp(reglist[j], arg1.name)) free_register();
                    } else {
                        if (Ast -> ast_function -> function[0] == '+') fprintf(outfile, "\tadd\t%s, %s\n", arg1.name, arg2.name);
                        else fprintf(outfile, "\tsub\t%s, %s\n", arg1.name, arg2.name);
                        strcpy (outreg.name, arg1.name);
                        outreg.type = 0;
                    }
                    for (int j = 0; j < 8; j++) if (!strcmp(reglist[j], arg2.name)) free_register();
                    free(arg1.name);
                    free(arg2.name);
                    break;
                }
                case '/' :
                case '*' :{
                    char *arg1 = compile(Ast -> ast_function -> body).name;
                    Ast -> ast_function -> body ++;
                    fprintf(outfile, "\tmov\trax, %s\n", arg1);
                    char *arg2 = compile (Ast -> ast_function -> body).name;
                    Ast -> ast_function -> body --;
                    if (Ast -> ast_function -> function[0] == '*') fprintf(outfile, "\tmul\t%s\n", arg2);
                    else fprintf(outfile, "\tmov\trdx, 0\n\tdiv\t%s\n", arg2);
                    outreg.type = 0;
                    strcpy (outreg.name, "rax");
                    for (int j = 0; j < 8; j++) if ((!strcmp(reglist[j], arg2)) || (!strcmp(reglist[j], arg1))) free_register();
                    free(arg1);
                    free(arg2);
                    break;
                }
                case '!' :
                case '=' :
                case '>' :
                case '<' :
                {
                    char *cmp = malloc (sizeof(*cmp) * 3);
                    if      ((Ast -> ast_function -> function[0] == '<') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "le");
                    else if (Ast -> ast_function  -> function[0] == '<') strcpy (cmp, "l");
                    else if ((Ast -> ast_function -> function[0] == '>') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "ge");
                    else if (Ast -> ast_function  -> function[0] == '>') strcpy (cmp, "g");
                    else if ((Ast -> ast_function -> function[0] == '=') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "e");
                    else if ((Ast -> ast_function -> function[0] == '!') && (Ast -> ast_function -> function[1] == '=')) strcpy (cmp, "ne");
                    else if (Ast -> ast_function  -> function[0] == '=')
                    {
                        int index = varindex (Ast -> ast_function -> body -> ast_identifier -> name);
                        int index_of_identifier = Ast -> ast_function -> body -> ast_identifier -> has_index;
                        Ast -> ast_function -> body ++;
                        if ((Ast -> ast_function -> body -> type    ==       ZNUMBER)
                            || (Ast -> ast_function -> body -> type == FUNCTION_CALL)
                            || (Ast -> ast_function -> body -> type ==  ZIDENTIFIER))
                        {
                            if (Ast -> ast_function -> body -> length == 1)
                            {
                                struct reg arg = compile (Ast -> ast_function -> body);
                                char *index_of_var = malloc (sizeof(*index_of_var) * 256);
                                if (index_of_identifier == 1) {
                                    Ast -> ast_function -> body --;
                                    strcpy(index_of_var, compile(Ast -> ast_function -> body -> ast_identifier -> index).name);
                                    Ast -> ast_function -> body ++;
                                } else strcpy(index_of_var, "0");
                                if ((symbol_table + index) -> type == -1)
                                {
                                    (symbol_table + index) -> array_length = 0;
                                    (symbol_table + index) -> type = arg.type;
                                    (symbol_table + index) -> is_static = 0;
                                }
                                else if (((symbol_table + index) -> array_length != 0) && (index_of_identifier == 0)) error("Assigning a single value to an array.");
                                else if ((symbol_table + index) -> type != 0) error("Changed type of variable");
                                if (arg.type == 0) fprintf (outfile, "\tmov\t[%s + 8 * %s], %s\n", (symbol_table + index) -> name, index_of_var, arg.name);
                                else fprintf(outfile, "\tmov\trdi, %s\n\tmov\trsi, %s\n\tmov\trdx, 256\n\tcall\tstrncpy\n", (symbol_table + index) -> name, arg.name);
                                free(index_of_var);
                                for (int j = 0; j < 8; j++) if (!strcmp(reglist[j], arg.name)) free_register();
                                free(arg.name);
                            }
                            else
                            {
                                for (int i = 0; i < Ast -> ast_function -> body -> length; i ++)
                                {
                                    char *element = compile(Ast -> ast_function -> body).name;
                                    fprintf(outfile, "\tmov\t[%s + %d], %s\n",
                                            (symbol_table + index) -> name,
                                            i * 8,
                                            element);
                                    free_register();
                                    Ast -> ast_function -> body -> ast_number ++;
                                    free(element);
                                }
                                Ast -> ast_function -> body -> ast_number -= Ast -> ast_function -> body -> length;
                                if ((symbol_table + index) -> type == -1)
                                {
                                    (symbol_table + index) -> array_length = Ast -> ast_function -> body -> length;
                                    (symbol_table + index) -> is_static    = 0;
                                    (symbol_table + index) -> type         = 0;
                                }
                            }
                        }
                        else if (Ast -> ast_function -> body -> type == ZSTRING)
                        {
                            int string_length = strlen(Ast -> ast_function -> body -> ast_string -> value);          // If it's a string first gets it size
                            if ((symbol_table + index) -> type == -1) (symbol_table + index) -> type = CHAR_LIST;    // Then if the variable haven't been initialize yet give it the string type
                            else if ((symbol_table + index) -> type != CHAR_LIST)                                    // If it has been initialized with another type, throw an error and exit
                                error("changing variable type.");
                            if ((symbol_table + index) -> array_length < string_length)
                                (symbol_table + index) -> array_length = string_length + 1;
                            for (int i = 0; i < string_length; i ++)
                                fprintf(outfile, "\tmov\tBYTE [%s + %d], %d\n",
                                        (symbol_table + index) -> name,
                                        i,
                                        Ast -> ast_function -> body -> ast_string -> value[i]);
                            fprintf(outfile, "\tmov\tBYTE [%s + %d], 0\n", (symbol_table + index) -> name, string_length);
                        }
                        free(cmp);
                        Ast -> ast_function -> body --;
                        break;

                    }
                    char *arg1 = compile (Ast -> ast_function -> body).name;
                    Ast -> ast_function -> body ++;
                    char *arg2 = compile (Ast -> ast_function -> body).name;
                    Ast -> ast_function -> body --;
                    fprintf(outfile,"\tcmp\t%s, %s\n\tj%s\t_true%d\n\tmov\trax, 0\n\tjmp\t_after%d\n_true%d:\n\tmov\trax, 1\n_after%d:\n",
                            arg1,
                            arg2,
                            cmp,
                            number_cmp,
                            number_cmp,
                            number_cmp,
                            number_cmp);
                    outreg.type = 2;
                    strcpy (outreg.name, "rax");
                    for (int j = 0; j < 8; j++) if ((!strcmp(reglist[j], arg2)) || (!strcmp(reglist[j], arg1))) free_register();
                    free(arg1);
                    free(arg2);
                    free (cmp);
                    number_cmp ++;
                    break;
                }
                default :
                    if (!strcmp("print", Ast -> ast_function -> function)) {
                        struct reg arg = compile(Ast -> ast_function -> body);
                        if (arg.type == 0)
                        {
                            fprintf(outfile, "\tpush\trbp\n"); /* Set up stack frame, must be alligned */
                            if ((Ast -> ast_function -> body -> type == ZNUMBER) && (Ast -> ast_function -> body -> length != 1))
                            {
                                (symbol_table + varind) -> is_static = 1;
                                strcat((symbol_table + varind) -> string, "[%d,");
                                for (int i = 0; i < Ast -> ast_function -> body -> length - 2; i++) strcat((symbol_table + varind) -> string, "%d,");
                                strcat((symbol_table + varind) -> string, "%d]");
                                varind ++;
                            }
                            else if ((Ast -> ast_function -> body -> type == ZIDENTIFIER) &&
                                     ((symbol_table + varindex (Ast -> ast_function -> body -> ast_identifier -> name)) -> array_length != 0))
                            {
                                (symbol_table + varind) -> is_static = 1;
                                sprintf((symbol_table + varind) -> name, "array_%d", number_array);
                                strcat((symbol_table + varind) -> string, "[%d, ");
                                for (int i = 0; i < (symbol_table + varindex (Ast -> ast_function -> body -> ast_identifier -> name)) -> array_length - 2; i++)
                                    strcat((symbol_table + varind) -> string, "%d, ");
                                strcat((symbol_table + varind) -> string, "%d]");
                                for (int i = 0; i < (symbol_table + varindex (Ast -> ast_function -> body -> ast_identifier -> name)) -> array_length; i++)
                                    fprintf(outfile, "\tmov\t%s, [%s + 8 * %d]\n",
                                            arg_func_list[i + 1],
                                            (symbol_table + varindex (Ast -> ast_function -> body -> ast_identifier -> name)) -> name,
                                            i );
                                (symbol_table + varind) -> At_the_end_0xA = 1;
                                fprintf(outfile, "\tmov\trdi, array_%d\n\txor rax, rax\n\tcall\tprintf wrt ..plt\n\txor\trax, rax\n", number_array);
                                number_array ++;
                                varind ++;
                            }
                            else fprintf(outfile, "\tmov\trsi, %s\n\tmov\trdi, int_to_str\n\txor rax, rax\n\tcall\tprintf wrt ..plt\n\txor\trax, rax\n", arg.name);
                            free_register();
                        }
                        else if (arg.type == 1) fprintf(outfile, "\tmov\trdi, %s\n\tcall\tputs\n", arg.name);
                        fprintf(outfile, "\tpop\trbp\n"); /* Restore stack */
                        free(arg.name);
                    }
                    else if (!strcmp("read", Ast -> ast_function -> function)) {
                        if (Ast -> ast_function -> body_length != 0) compile(Ast -> ast_function -> body);
                        outreg.type = CHAR_LIST;
                        strcpy(outreg.name, "out_buffer");
                        fprintf(outfile,
                                "\tmov\teax, 3\n"
                                "\tmov\tebx, 0\n"
                                "\txor\tecx, ecx\n"
                                "\tmov\tecx, out_buffer\n"
                                "\tmov\tedx, 256\n"
                                "\tint\t80h\n");
                    } else if (!strcmp("int", Ast -> ast_function -> function)) {
                        if (Ast -> ast_function -> body -> type == ZIDENTIFIER) // Return an error if the input is an array
                            if ((symbol_table + varindex (Ast -> ast_function -> body -> ast_identifier -> name)) -> array_length != 0)
                                error("Can't convert an array to an integer");
                        struct reg arg = compile(Ast -> ast_function -> body);
                        outreg.type = INTEGER;
                        if (arg.type == INTEGER) {
                            warning("Converting an integer to an integer, useless");
                            strcpy(outreg.name, arg.name);
                        } else {
                            fprintf(outfile,
                                    "\tmov\trdi,%s\n"
                                    "\tcall\tatoi\n"
                                    , arg.name);
                            strcpy(outreg.name, "rax");
                        }
                        free(arg.name);
                    } else if (is_in_strings(Ast -> ast_function -> function, custom_functions, number_functions)) {
                        char *args = malloc(256 * Ast -> ast_function -> body_length * sizeof(*args)); // allocate memory for the arguments
                        Ast -> ast_function -> body += Ast -> ast_function -> body_length - 1;
                        for (int i = 0; i < Ast -> ast_function -> body_length; i++) { // Get all the arguments and save them in the "args" list
                            struct reg arg1 = compile(Ast -> ast_function -> body);
                            strcpy(args + i * 256, arg1.name);
                            free(arg1.name);
                            Ast -> ast_function -> body --;
                        }
                        save_registers();
                        Ast -> ast_function -> body ++;
                        for (int i = 0; i < Ast -> ast_function -> body_length; i ++)
                        {
                            fprintf(outfile, "\tpush\t%s\n", (args + i * 256)); // Push the arguments to the stack
                            for (int j = 0; j < 8; j++) if (!strcmp(reglist[j], (args + i * 256))) free_register(); // We don't need allocated registers anymore since we've pushed them to the stack
                        }
                        fprintf(outfile, "\tcall\t_%s\n", Ast -> ast_function -> function);
                        for (int i = 0; i < Ast -> ast_function -> body_length; i++) fprintf(outfile, "\tpop\trcx\n"); // Remove the arguments from the stack
                        int reg = new_register(); // Allocate a register for the output
                        strcpy(outreg.name, reglist[reg]);
                        restore_registers();
                        fprintf(outfile, "\tmov\t%s, rax\n", reglist[reg]); // Put the result in the register
                        fprintf(outfile, "\tpop\trax\n"); // Restore registers on the stack
                        outreg.type = 0;
                        free(args);
                    }
            }
            break;
        case ZNUMBER : // If the AST is a number allocate a register and put the number in it
        {
            int reg = new_register();
            fprintf(outfile, "\tmov\t%s, %d\n", reglist[reg], Ast -> ast_number -> value);
            strcpy (outreg.name, reglist[reg]);
            outreg.type = 0;
            break;
        }
        case ZSTRING :
            (symbol_table + varind) -> is_static = 1;
            char *str_name = malloc (sizeof(*str_name) * 256);
            sprintf(str_name, "str%d", number_stings);
            number_stings ++;
            strcpy((symbol_table + varind) -> name, str_name);
            strcpy((symbol_table + varind) -> string, Ast -> ast_string -> value);
            (symbol_table + varind) -> type = 1;
            varind ++;
            (symbol_table + varind) -> At_the_end_0xA = 0;
            outreg.type = 1;
            strcpy (outreg.name, str_name);
            free(str_name);
            break;
        case VARDECLARATION :
            strcpy((symbol_table + varind) -> name, Ast -> ast_vardeclaration -> name);
            (symbol_table + varind) -> type = -1;
            (symbol_table + varind) -> array_length = 0;
            varind ++;
            break;
        case IFSTATEMENT :
        {
            nubmer_structures ++;
            int i = nubmer_structures - 1;
            struct reg condition = compile (Ast -> ast_if -> condition);
            fprintf(outfile, "\tcmp\t%s, 0\n", condition.name);
            free(condition.name);
            if ((Ast + 1) -> type == ELSESTATEMENT) fprintf(outfile, "\tje\t_else%d\n", i);
            else fprintf(outfile, "\tje\t_aft%d\n", i);
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
            if ((Ast + 1) -> type == ELSESTATEMENT)
            {
                struct reg tmpreg;
                tmpreg.name = malloc (sizeof(*tmpreg.name) * 256);
                fprintf(outfile, "\tjmp\t_aft%d\n_else%d:\n", i, i);
                Ast ++;
                for (int i = 0; i < Ast -> ast_else -> body_length; i ++)
                {
                    struct reg comp = compile (Ast -> ast_else -> body);
                    if (comp.type != -1) strcpy (tmpreg.name, comp.name);
                    Ast -> ast_else -> body ++;
                    free (comp.name);
                }
                Ast -> ast_else -> body -= Ast -> ast_else -> body_length;
                Ast --;
                if (outreg.type != -1) fprintf(outfile, "\tmov\t%s, %s\n", outreg.name, tmpreg.name);
                free(tmpreg.name);
            }
            fprintf(outfile, "_aft%d:\n", i);
            break;
        }
        case WHILESTATEMENT :
        {
            fprintf(outfile, "_start%d:\n",  nubmer_structures);
            int act_num = nubmer_structures;
            nubmer_structures ++;
            char *condition = malloc (sizeof(*condition) * 256);
            strcpy (condition, compile (Ast -> ast_if -> condition).name);
            fprintf(outfile, "\tcmp\t%s, 0\n\tje\t_aft%d\n", condition, act_num);
            for (int i = 0; i < Ast -> ast_while -> body_length; i ++)
            {
                compile (Ast -> ast_while -> body);
                Ast -> ast_while -> body ++;
            }
            Ast -> ast_while -> body -= Ast -> ast_while -> body_length;
            fprintf(outfile, "\tjmp\t_start%d\n_aft%d:\n", act_num, act_num);
            break;
        }
        case ZIDENTIFIER :
        {
            int index = varindex (Ast -> ast_identifier -> name);
            int reg = new_register();
            if ((symbol_table + index) -> type == -1) error("used unisialized variable");
            else if ((symbol_table + index) -> type == INTEGER) {
                if (Ast -> ast_identifier -> has_index == 1) {
                    char *index = compile(Ast -> ast_identifier -> index).name;
                    fprintf(outfile, "\tmov\t%s, [%s + %s * 8]\n", reglist[reg], Ast -> ast_identifier -> name, index);
                    free_register();
                    free(index);
                } else fprintf(outfile, "\tmov\t%s, [%s]\n", reglist[reg], Ast -> ast_identifier -> name);
            } else {
                if (Ast -> ast_identifier -> has_index == 1) {
                    char *index = compile(Ast -> ast_identifier -> index).name;
                    int reg2 = new_register();
                    fprintf(outfile, "\tmov\t%sb, [%s + %s]\n\tmov BYTE\t[char_buffer], %sb\n\tmov\t%s, char_buffer\n",
                            reglist[reg2],
                            Ast -> ast_identifier -> name,
                            index,
                            reglist[reg2],
                            reglist[reg]);
                    free_register();
                    free(index);
                }
                else fprintf(outfile, "\tmov\t%s, %s\n", reglist[reg], Ast -> ast_identifier -> name);
            }
            strcpy (outreg.name, reglist[reg]);
            outreg.type = (symbol_table + index) -> type;
            break;
        }
        case ELSESTATEMENT : break;
        case ELIFSTATEMENT : break;
        case AST :
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
            break;
        }
        case REG:
            break;
        case STACK_POS:
        {
            int reg = new_register();
            fprintf(outfile, "\tmov\t%s, [rsp + %d]\n", reglist[reg], 8 * (Ast -> stack_pos + 1));
            strcpy(outreg.name, reglist[reg]);
            outreg.type = 0;
            break;
        }
    }
    if ((outreg.type != -1) && (Ast -> is_negative)) fprintf(outfile, "\tmov\trcx, -1\n\tmov\trax, %s\n\tmul\trcx\n\tmov\t%s, rax\n", outreg.name, outreg.name);
    return outreg;
}

/*
 * The epilog of our program
 * This function outputs to the output file the content that should be at the
 * end like the static data and the needed space in the .bss section.
 */
void
epilog(int is_lib)
{
    if (!is_lib) fprintf(outfile, "\tmov\teax,1\n\tmov\tebx,0\n\tint\t80h\n");
    used_registers = 0;
    for (int i = 0; i < varind ; i ++)
    {
        if ((symbol_table + i) -> type == 3)
        {
            fprintf(outfile, "_%s:\n", (symbol_table + i) -> name);
            struct reg final;
            final.name = malloc(256 * sizeof(*final.name));
            for (int j = 0; j < (symbol_table + i) -> ast -> ast_functiondeclaration -> body_length; j++)
            {
                struct reg comp = compile((symbol_table + i) -> ast -> ast_functiondeclaration -> body);
                if (comp.type != -1)
                {
                    final.type = comp.type;
                    strcpy (final.name, comp.name);
                }
                free(comp.name);
                (symbol_table + i) -> ast -> ast_functiondeclaration -> body ++;
            }
            fprintf(outfile, "\tmov\trax, %s\n", final.name);
            fprintf(outfile, "\tret\n");
            (symbol_table + i) -> ast -> ast_functiondeclaration -> body -= (symbol_table + i) -> ast -> ast_functiondeclaration -> body_length;
            freeall((symbol_table + i) -> ast);
            free((symbol_table + i) -> ast);
            free(final.name);
        }
    }
    if (!is_lib)
    {
        fprintf(outfile, "section .data\n");
        for (int i = 0; i < varind ; i ++)
        {
            if ((symbol_table + i) -> is_static == 1)
            {
                char *end = malloc(10 * sizeof(end));
                if ((symbol_table + i) -> At_the_end_0xA == 1) strcpy(end, "0xA,0");
                else strcpy(end, "0");
                fprintf(outfile, "\t%s:\t db '%s', %s\n\t%s_len:\tequ $-%s\n",
                        (symbol_table + i) -> name,
                        (symbol_table + i) -> string,
                        end,
                        (symbol_table + i) -> name,
                        (symbol_table + i) -> name);
                free(end);
            }
        }
        fprintf(outfile, "\tint_to_str:\t db '%%d',0xA\nsection .bss\n\tchar_buffer:\tresb 2\n\tbuffer:\tresb 256\n\tout_buffer:\tresb 256\n");
        for (int i = 0; i < varind ; i ++)
        {
            if (((symbol_table + i) -> is_static == 0) && ((symbol_table + i) -> type == 0))
            {
                if ((symbol_table + i) -> array_length == 0) fprintf(outfile, "\t%s:\tresq 1\n",
                                                                     (symbol_table + i) -> name);
                else if ((symbol_table + i) -> array_length != 0) fprintf(outfile, "\t%s:\t resq %d\n",
                                                                          (symbol_table + i) -> name, (symbol_table + i) -> array_length);
            }
            else if (((symbol_table + i) -> is_static == 0) && ((symbol_table + i) -> type == CHAR_LIST)){
                fprintf(outfile, "\t%s:\tresb 256\n", (symbol_table + i) -> name);
            }
        }
    }
}

int
main ( int argc, char *argv[] )
{
    if (argc < 2) exit(1); // If we don't have a file to compile exit.
    char *linker       = malloc(256 * sizeof(*linker)),
         *linker_flags = malloc(256 * sizeof(*linker_flags)),
         *outcommand   = malloc (sizeof(*outcommand) * 256);
    int is_lib         = 0;
    if (USE_MUSL)      strcpy(linker, "musl-gcc");
    else               strcpy(linker, "gcc");
    if (STATIC_LINKED) strcpy(linker_flags, "-flto -static -Os -s -Wl,--gc-sections -static");
    else               strcpy(linker_flags, "-flto -static -Os -s -Wl,--gc-sections");
    if (argc >= 3) {
        if (!strcmp(argv[2], "-lib")) is_lib = 1;
        if (!strcmp(argv[2], "-o")) {
            if (argc < 4) error("the -o option need an argument");
            sprintf(outcommand, "nasm -f elf64 ./out.asm && %s -o %s ./out.o  %s && rm out.asm && rm out.o", linker, argv[3], linker_flags);
        } else sprintf(outcommand, "nasm -f elf64 ./out.asm && %s %s ./out.o -o out && rm out.asm && rm out.o", linker, linker_flags);
    } else sprintf(outcommand, "nasm -f elf64 ./out.asm && %s %s ./out.o -o out && rm out.asm && rm out.o", linker, linker_flags);
    fp1     = fopen (argv[1], "r");
    outfile = fopen ("out.asm", "w");
    if (!is_lib) fprintf(outfile,
                         "section\t.text\nglobal\tmain\n\textern"
                         "\tprintf\n\textern\tputs\n\textern\tstrcat\n"
                         "\textern\tstrncpy\n\textern\tatoi\nmain:\n\tmov BYTE\t[char_buffer + 1], 0\n"); // Print the necessary components for the beginning of a nasm program
    struct parse outfinal;
    symbol_table  = malloc(SYMBOL_TABLE_SIZE * sizeof(struct variable)); // Initialize the symbol table and the ASTs of the program
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) (symbol_table + i) -> is_static = 0;
    outfinal.size = 0;
    outfinal.body = malloc(20 * sizeof(struct leaf));
    while(1)
    {
        struct token *tokens;
        tokens = malloc(100 * sizeof(struct token));
        struct parse out = parsestatement(lexer(fp1, 0, tokens), "\n", -1);
        for (int i = 0; i < out.size; i ++){
            move_ast(out.body, outfinal.body, i, outfinal.size);
            outfinal.size ++;
        }
        free(tokens);
        if (out.size == -1) break;
        free(out.body);
    }
    fclose(fp1);
    for (int i = 0; i < outfinal.size; i++) {
        check(outfinal.body);
        free(compile(outfinal.body).name);
        freeall(outfinal.body);
        outfinal.body ++;
    }
    outfinal.body -= outfinal.size;
    epilog(is_lib);
    free(symbol_table); // Free malloc'd memory
    free(outfinal.body);
    fclose(outfile);    // Close the output file
    if (!is_lib) system(outcommand); //Assemble and link the produced program if it's not a library
    printf("\033[1;32mDone!\033[0m\n");
    return 0;
}
