#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum type {operator,
           separator,
           identifier,
           number};
struct token {
    enum type type;
    char value[50];
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

struct token * lexer(FILE *fp1){
    struct token *tokens;
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
            puts((tokens+k)->value);
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
            puts((tokens+k)->value);
            k++;
        }
        else if (isinchars(operators, c)){
            (tokens+k)->type = 0;
            conv[0] = c;
            strcpy((tokens+k)->value, conv);
            puts((tokens+k)->value);
            k++;
        }
        else if (isinchars(symbols, c)){
            (tokens+k)->type = 1;
            conv[0] = c;
            strcpy((tokens+k)->value, conv);
            puts((tokens+k)->value);
            k++;
        }
        else if (c == '\n'){
            (tokens+k)->type = 1;
            strcpy((tokens+k)->value, "terminator");
            puts((tokens+k)->value);
            k++;
        }
    }
    return tokens;
    free(tokens);
}

void parser(){

}
int main( int argc, char *argv[] ){
    FILE *fp1 = fopen (argv[1], "r");
    lexer(fp1);
    fclose(fp1);
    return 0;
}
