#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum type {operator,
           separator,
           identifier,
           number};
enum var_types{integer,
               floating,
               string,
               charachter,
               boolean};
struct token {
    enum type type;
    char value[50];
};
struct lexline {
    struct token *tokens;
    int size;
};
struct var {
    enum var_types type;
    char value[50];
    int init_value;
};
char symbols[10] = {'(',')','{','}',';','"','[',']',',','#'};
char operators[8] = {'>','<', '=', '!', '+', '/', '-', '%'};
char *keywords[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"};

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

struct lexline lexer(FILE *fp1){
    struct token *tokens;
    struct lexline lex;
    tokens = (struct token*) malloc(20 * sizeof(struct token));
    char c = 'a';
    int i = 0, j = 0, k = 0, l = 0;
    char buffer[100];
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
            (tokens+k)->type = 2;
            strcpy((tokens+k)->value, buffer);
            k++;
        }
        else if (isdigit(c)){
            j = ftell(fp1);
            i = 0;
            while ((c != ' ') && (c != '\n')){
                i++;
                c = fgetc(fp1);
                if(!isdigit(c)){
                    (tokens+k)->type = 2;
                }
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(buffer, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            if ((tokens+k)->type != 2){
                (tokens+k)->type = 3;
            }
            strcpy((tokens+k)->value, buffer);
            k++;
        }
        else if (isinchars(operators, c)){
            (tokens+k)->type = 0;
            conv[0] = c;
            strcpy((tokens+k)->value, conv);
            k++;
        }
        else if (isinchars(symbols, c)){
            (tokens+k)->type = 1;
            conv[0] = c;
            strcpy((tokens+k)->value, conv);
            k++;
        }
        else if (c == '\n'){
            (tokens+k)->type = 1;
            tokens = (struct token*) realloc(tokens, k * sizeof(struct token));
            lex.size=(k);
            lex.tokens=tokens;
            return lex;
            free(tokens);
        }
    }
    exit(0);
}

void parser(struct lexline lex){
    struct token *tokens = lex.tokens;
    for (int i = lex.size; i > -1; i--){
        switch ((tokens+i)->type){
            case operator:
                switch((tokens+i)->value[0]){
                    case '=':

                }
                break;
            case separator:
                break;
            case identifier:
                break;
            case number:
                break;
        }
        puts((tokens+i)->value);
    }
}
int main( int argc, char *argv[] ){
    FILE *fp1 = fopen (argv[1], "r");
    while(1){
        parser(lexer(fp1));
    }
    fclose(fp1);
    return 0;
}
