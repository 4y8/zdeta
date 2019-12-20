#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum type{operator, terminator, identifier, number};
struct token {
    enum type type;
    char value[50];
};

char symbols[9] = {'(',')','{','}',';','"','[',']',','};
char operators[8] = {'>','<', '=', '!', '+', '/', '-', '%'};
char *funs[13] = {"let", "fun", "print", "while", "if", "else",
                  "elif", "var", "swap", "case", "switch", "iter"}; // The list of all functions
struct token *tokens;

int isfun(char in[]){
    for(int i = 0; i < 12; i++){
        if (strcmp(funs[i], in) == 0){
            return 1;
        }
    }
    return 0;
}

int isinchars(char in[], char check){
    for(int i = 0; i < 13; i++){
        if (in[i] == check){
            return 1;
        }
    }
    return 0;
}

void lexer(FILE *fp1){
    tokens = (struct token*) malloc(10 * sizeof(struct token));
    char c = 'a';
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    char buffer[100];
    char conv[2] = {'a', '\0'};
    while(c != EOF){
        c = fgetc(fp1);
        if (isalpha(c)){
            j = ftell(fp1);
            i = 0;
            while (c != ' '){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(buffer, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            (tokens+k)->type = 2;
            strcpy((tokens+k)->value, buffer);
            k ++;
        }
        else if (isdigit(c)){
            j = ftell(fp1);
            i = 0;
            while (c != ' '){
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
            k ++;
        }
        else {
            if (isinchars(operators, c)){
                strcpy(pars[k].type, "Opp");
                if(c == '/'){
                    c = fgetc(fp1);
                    if(c == '/'){
                        while (c != '\n'){
                            c = fgetc(fp1);
                        }
                        fseek(fp1, -1, SEEK_CUR);
                    }
                    else{
                        conv[0] = '/';
                        strcpy(pars[k].name, conv);
                    }
                }
                else{
                    conv[0] = c;
                    strcpy(pars[k].name, conv);
                }
                k ++;
            }
            else if (isinchars(symbols, c)){
                strcpy(pars[k].type, "Symb");
                switch(c){
                    case '"':
                        strcpy(pars[k].name, "quote");
                        break;
                    case '[':
                        strcpy(pars[k].name, "leftsquarebracket");
                        break;
                    case ']':
                        strcpy(pars[k].name, "rightsquarebracket");
                        break;
                    case ';':
                        strcpy(pars[k].name, "semicolon");
                        break;
                    case '(':
                        strcpy(pars[k].name, "leftparenthesis");
                        break;
                    case ')':
                        strcpy(pars[k].name, "rightparenthesis");
                        break;
                    case ',':
                        strcpy(pars[k].name, "coma");
                        break;
                    case '=':
                        strcpy(pars[k].name, "equal");
                        break;
                    case '<':
                        strcpy(pars[k].name, "lowerthan");
                        break;
                    case '>':
                        strcpy(pars[k].name, "greaterthan");
                        break;
                    case '!':
                        strcpy(pars[k].name, "exclam");
                        break;
                    case '%':
                        strcpy(pars[k].name, "mod");
                        break;
                }
                k ++;
            }
        }
        if(c == '\n'){
            if(strcmp(pars[k - 1].name, "Ter") != 0){
                strcpy(pars[k].type, "Ter");
                strcpy(pars[k].name, "Ter");
                k ++;
                i = 0;
                strcpy(pars[k].type, "Spc");
                if (isspace(c)){
                    while(isspace(c)){
                        c = fgetc(fp1);
                        i ++;
                    }
                    fseek(fp1, -1, SEEK_CUR);
                    sprintf(pars[k].name, "%d", i);
                }
                k ++;
            }
        }
    }
}


int main( int argc, char *argv[] ){
    FILE *fp1 = fopen (argv[1], "r");
    FILE *fp2 = fopen("testhello.c", "w+");
    fputs ("#include <stdio.h>\n", fp2);
    fputs ("int main(){\n", fp2);
    char c = 'a';
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    lexer(fp1);
}
