#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct lex {char type[5]; char name[50];};
struct var {char name[50]; int type; int len;};
char symb[12] = {'(',')','{','}',';','"','[',']',',','>','<','='}; // The list of all symbols
char *funs[13] = {"let", "fun", "print", "while", "if", "else", "elif", "var", "end", "swap", "case", "switch", "iter"}; // The list of all functions
char opps[4] = {'+', '/', '-', '%'};
struct lex pars[300];
struct var vars[50];
char conv[2] = {'a', '\0'};
char str[100];
int indent = 0; // A variable to keep track of the indentation

int isfun(char in[]){
    for(int i = 0; i < 13; i++){
        if (strcmp(funs[i], in) == 0){
            return 1;
        }
    }
    return 0;
}

int isvar(char in[]){
    for(int i = 0; i < 50; i++){
        if (strcmp(vars[i].name, in) == 0){
            return 1;
        }
    }
    return 0;
}
int isinchars(char in[], char check){
    for(int i = 0; i < 12; i++){
        if (in[i] == check){
            return 1;
        }
    }
    return 0;
}

int varind (char in[]){
    for(int i = 0; i < 50; i++){
        if (strcmp(vars[i].name, in) == 0){
            return i;
        }
    }
    return -1;
}

void error(char in[]){
    printf("\033[1;31m Error:\033[0m %s\n", in);
    exit(0); // Exit the program if there is an error
}

int print(FILE *fp2, int i){
    i ++;
    int j = 0;
    if (strcmp(pars[i].name, "quote") == 0){
        i++;
        fputs("    puts(", fp2);
        fputc('"', fp2);
        while (pars[i].name[0] != 'q'){
            fputs(pars[i].name, fp2);
            fputs(" ", fp2);
            i++;
        }
        fputc('"', fp2);
        fputs(");\n", fp2);
    }
    else if (isvar(pars[i].name) == 1){
        j = varind(pars[i].name);
        if (vars[j].type == 0){
            fprintf(fp2, "    printf(%c%s%c,%s);\n", '"', "%d", '"', vars[j].name);
        }
        else if (vars[j].type == 1){
            fprintf(fp2, "    for(int z = 0; z < %d; z++){\n        printf(%c%cd %c, %s[z]);\n    }\n", vars[j].len, '"', '%','"', vars[j].name);
        }
    }
    else {
        error("Printing a non variable or string element");
    }
    return i;
}

void createvar(FILE *fp2, int index, int ind){
    strcpy(vars[index].name, pars[ind].name);
    int len = 0;
    fputs("    int ", fp2);
    fputs(pars[ind].name, fp2);
    ind += 2;
    if (strcmp(pars[ind].name, "leftsquarebracket") == 0){
        fputs(" [", fp2);
        ind ++;
        while (strcmp(pars[ind].name, "rightsquarebracket") != 0){
            if (strcmp(pars[ind].name, "coma") == 0){
                ind ++;
            }
            else if (strcmp(pars[ind].type, "Num") == 0){
                ind ++;
                len ++;
            }
        }
        vars[index].len = len;
        fprintf(fp2,"%d]",len);
        vars[index].type = 1;
    }
    else{
        vars[index].type = 0;
    }
    fputs(";\n", fp2);
}

int assignvar(FILE *fp2, int index, int find){
    find ++;
    int i = 0;
    if (strcmp(pars[find].name, "leftsquarebracket") == 0){
        find ++;
        while (strcmp(pars[find].name, "rightsquarebracket") != 0){
            if (strcmp(pars[find].name, "coma") == 0){
                find ++;
            }
            else if (strcmp(pars[find].type, "Num") == 0){
                fprintf(fp2, "    %s[%d] = %s;\n",vars[index].name, i,pars[find].name );
                find ++;
                i ++;
            }
        }
    }
    else{
        fprintf(fp2,"    %s =", vars[index].name);
        while(strcmp(pars[find].name, "Ter") != 0){
            if (strcmp(pars[find].type, "Symb") == 0){
                if(strcmp(pars[find].name, "leftsquarebracket") == 0){
                    fputc('[',fp2);
                }
                else if(strcmp(pars[find].name, "rightsquarebracket") == 0){
                    fputc(']',fp2);
                }
            }
            else{
                fprintf(fp2, "%s", pars[find].name);
            }
            find ++;
        }
        fputs(";\n", fp2);
    }
    return find;
}

int assignarray(FILE *fp2, int find){
    fprintf(fp2, "    %s[%s] = ", pars[find - 4].name, pars[find - 2].name);
    while (strcmp(pars[find].name, "Ter") != 0){
        if (strcmp(pars[find].type, "Symb") == 0){
            if(strcmp(pars[find].name, "leftsquarebracket") == 0){
                fputc('[',fp2);
            }
            else if(strcmp(pars[find].name, "rightsquarebracket") == 0){
                fputc(']',fp2);
            }
        }
        else{
            fprintf(fp2, "%s", pars[find].name);
        }
        find ++;
    }
    find --;
    fputs(";\n", fp2);
    return find;
}

// Here is the increment system
void increment(FILE *fp2, char val[]){
    if(isvar(val)){ // Check if we increment a variable
        fprintf(fp2,"    %s++;\n", val);
    }
    else{ // Else we print an error
        error("Incrementing a non variable element");
    }
}

// Here is the increment system
void decrement(FILE *fp2, char val[]){
    if(isvar(val)){ // Check if we decrement a variable
        fprintf(fp2,"    %s++;\n", val);
    }
    else{ // Else we print an error
        error("Decrementing a non variable element");
    }
}

// Here is the array system
int arrayelement(FILE *fp2, int index, int j){
    int i = varind(pars[index].name);
    // First we check if we have an array
    if (i == -1){
        puts ("Error: attributing index to non-list element"); // If no we print an error message
    }
    else{
        index++;
        if (strcmp(pars[index].name, "leftsquarebracket") == 0){ // Check if the variabiable is followed by a left square bracket
            index ++;
            if (strcmp(pars[index].type, "Num") == 0){
                index ++;
                if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                    fprintf(fp2, "%s[%s]", pars[index - 3].name, pars[index - 1].name);
                }
                else{
                    error("Probably missing a ']'"); // If there is no right square bracket print an error message
                }
            }
            else if (isvar(pars[index].name)){
                index ++;
                if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                    fprintf(fp2, "%s[%s]", pars[index - 3].name, pars[index - 1].name);
                }
                else if (strcmp(pars[index].type, "Opp") == 0){
                    switch (pars[index].name[0]){
                        case '+':
                            index ++;
                            if(pars[index].name[0] == '+'){
                                index++;
                                if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                                    fprintf(fp2, "%s[%s + 1]", pars[index - 5].name, pars[index - 3].name);
                                }
                                else{
                                    error("Probably missing a ']'"); // If there is no right square bracket print an error message
                                }
                            }
                            else{
                                index ++;
                                if ((strcmp(pars[index].type, "Num") || (isvar(pars[index].name)))){
                                    index ++;
                                    if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                                        fprintf(fp2, "%s[%s + %s]", pars[index - 5].name, pars[index - 3].name, pars[index - 1].name);
                                    }
                                    else{
                                        error("Probably missing a ']'"); // If there is no right square bracket print an error message
                                    }
                                }
                            }
                            break;
                        case '-':
                            index ++;
                            if(pars[index].name[0] == '-'){
                                index++;
                                if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                                    fprintf(fp2, "%s[%s - 1]", pars[index - 5].name, pars[index - 3].name);
                                }
                                else{
                                    error("Probably missing a ']'"); // If there is no right square bracket print an error message
                                }
                            }
                            else{
                                index ++;
                                if ((strcmp(pars[index].type, "Num") || (isvar(pars[index].name)))){
                                    index ++;
                                    if (strcmp(pars[index].name, "rightsquarebracket") == 0){
                                        fprintf(fp2, "%s[%s + %s]", pars[index - 5].name, pars[index - 3].name, pars[index - 1].name);
                                    }
                                    else{
                                        error("Probably missing a ']'"); // If there is no right square bracket print an error message
                                    }
                                }
                            }
                            break;
                    }
                }
                else{
                    error("Probably missing a ']'"); // If there is no right square bracket print an error message
                }
            }
            else{
                error("non-valid array index"); // Print an error the index is not a number nor a variable
            }
        }
        else{
            error("TODO");
        }
    }
    return index;
}

int structure(FILE *fp2, int ind, char name[]){
    int j = ftell(fp2);
    ind ++;
    int i = varind(pars[ind].name);
    if ((1 == vars[i].type) && (i != -1)){
        fprintf(fp2,"    %s(", name);
        ind = arrayelement(fp2, ind, j);
        ind ++;
        if (strcmp(pars[ind].name, "equal") == 0){
            fputs(" == ", fp2);
        }
        else if (strcmp(pars[ind].name, "lowerthan") == 0){
            fputs(" < ", fp2);
        }
        else if (strcmp(pars[ind].name, "greaterthan") == 0){
            fputs(" > ", fp2);
        }
        ind ++;
        i = varind(pars[ind].name);
        if (1 == vars[i].type){
            ind = arrayelement(fp2, ind, j);
        }
        else{
            fputs(pars[ind].name, fp2);
        }
        fputs("){\n", fp2);
    }
    else if ((vars[i].type == 0) || (strcmp(pars[ind].type, "Num") == 0)){
        ind++;
        if(strcmp(pars[ind].type, "Symb") == 0){
            ind --;
            fprintf(fp2,"    %s(", name);
            if ((strcmp(pars[ind].type, "Num") == 0) || (isvar(pars[ind].name))){
                fputs(pars[ind].name,fp2);
            }
            ind ++;
            if (strcmp(pars[ind].name, "equal") == 0){
                fputs(" == ", fp2);
            }
            else if (strcmp(pars[ind].name, "lowerthan") == 0){
                fputs(" < ", fp2);
            }
            else if (strcmp(pars[ind].name, "greaterthan") == 0){
                fputs(" > ", fp2);
            }
            ind ++;
            if ((strcmp(pars[ind].type, "Num") == 0) || (isvar(pars[ind].name))){
                fputs(pars[ind].name,fp2);
            }
            fputs("){\n", fp2);
        }
    }
    return ind;
}

void ellse(){

}
int main( int argc, char *argv[] ){
    FILE *fp1 = fopen (argv[1], "r");
    FILE *fp2 = fopen("testhello.c", "w+");
    fputs ("#include <stdio.h>\n", fp2);
    fputs ("int main(){\n", fp2);
    char c;
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    while(c != EOF){
        c = fgetc(fp1);
        if (isalpha(c)){
            j = ftell(fp1);
            i = 0;
            while (isalpha(c)){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(str, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            if (isfun(str) == 1){
                strcpy(pars[k].type, "Fun");
            }
            else{
                strcpy(pars[k].type, "Str");
            }
            strcpy(pars[k].name, str);
            k ++;
        }
        else if (isdigit(c)){
            j = ftell(fp1);
            i = 0;
            while (isdigit(c)){
                i++;
                c = fgetc(fp1);
            }
            l = ftell(fp1);
            fseek(fp1, j - 1, SEEK_SET);
            fgets(str, i + 1, fp1);
            fseek(fp1, l - 1, SEEK_SET);
            strcpy(pars[k].type, "Num");
            strcpy(pars[k].name, str);
            k ++;
        }
        else {
            if (isinchars(opps, c)){
                strcpy(pars[k].type, "Opp");
                if(c == '/'){
                    c = fgetc(fp1);
                    if(c == '/'){
                        while (c != '\n'){
                            c = fgetc(fp1);
                        }
                        strcpy(pars[k].type, "Ter");
                        strcpy(pars[k].name, "Ter");
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
            else if (isinchars(symb, c)){
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
                }
                k ++;
            }
        }
        if(c == '\n'){
            if(strcmp(pars[k - 1].name, "Ter") != 0){
                strcpy(pars[k].type, "Ter");
                strcpy(pars[k].name, "Ter");
                k ++;
            }
        }
        else if(c == '\t'){
            i = 0;
            strcpy(pars[k].type, "Tab");
            while(c == '\t'){
                i ++;
                c = fgetc(fp1);
            }
            sprintf(pars[k].name, "%d", i);
            fseek(fp1, -1, SEEK_CUR);
            k ++;
        }
        else if((isspace(c)) && (strcmp(pars[k - 1].name, "Ter") == 0)){
            i = 0;
            strcpy(pars[k].type, "Spc");
            while(isspace(c)){
                c = fgetc(fp1);
                i ++;
            }
            fseek(fp1, -1, SEEK_CUR);
            sprintf(pars[k].name, "%d", i);
            k ++;
        }
    }
    k = 0;
    for (i = 0; i < 200; i++){
        switch (pars[i].type[0]){
            case 'F':
                if (strcmp(pars[i].name, "print") == 0){
                    i = print(fp2, i);
                }
                else if (strcmp(pars[i].name, "if") == 0){
                    i = structure(fp2, i, "if");
                }
                else if (strcmp(pars[i].name, "end") == 0){
                    fputs("    }\n", fp2);
                }
                else if (strcmp(pars[i].name, "while") == 0){
                    i = structure(fp2, i, "while");
                }
                break;
            case 'S':
                if(isvar(pars[i].name)){
                    if(pars[i + 1].name[0] == '+'){
                        if(pars[i + 1].name[0] == '+'){
                            increment(fp2,pars[i].name);
                        }
                    }
                    else if(pars[i + 1].name[0] == '-'){
                        if(pars[i + 1].name[0] == '-'){
                            decrement(fp2,pars[i].name);
                        }
                    }
                    else if(strcmp(pars[i + 1].name, "equal") == 0){
                        j = varind(pars[i].name);
                        i ++;
                        i = assignvar(fp2, j, i);
                        i++;
                    }
                }
                else if (strcmp(pars[i].name, "equal") == 0){
                    if (strcmp(pars[i - 1].type, "Str") == 0){
                        if(isvar(pars[i].name) == 0){
                            createvar(fp2, k, i - 1);
                            i = assignvar(fp2, k, i);
                            k++;
                        }
                        else{
                            j = varind(pars[i].name);
                            i ++;
                            i = assignvar(fp2, j, i);
                            i ++;
                        }
                    }
                    else if(strcmp(pars[i - 1].name, "rightsquarebracket") == 0){
                        i = assignarray(fp2, i);
                    }
                }
                break;
        }
    }
    /*for(i = 0; i < 200; i++){
        puts(pars[i].name);
    }*/
    fclose(fp1);
    fputs("    return 0;\n", fp2);
    fputs("}\n", fp2);
    fclose(fp2);
    return 0;
}
