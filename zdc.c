#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct lex {char type[5]; char name[50];};
struct var {char name[50]; int type; int len;};
char symb[10] = {'(',')','{','}',';','"','[',']'};
char *funs[9] = {"let", "fun", "print", "while", "if", "else", "elif", "var", "end"};
char opps[7] = {'+', '/', '-', '=', '%','>','<'};
struct lex pars[200];
struct var vars[50];
char conv[2] = {'a', '\0'};
char str[100];

int isfun(char in[]){
    for(int i = 0; i < 9; i++){
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
    for(int i = 0; i < 10; i++){
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

int print(FILE *fp2, int i){
    i ++;
    int j = 0;
    if (strcmp(pars[i].name, "quote") == 0){
        i++;
        fputs("    puts(", fp2);
        fputc('"', fp2);
        while (pars[i].name[0] != 'q') {
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
            fputs("    printf(", fp2);
            fputc('"', fp2);
            if(vars[j].type == 0 ){
                fputs("%d", fp2);
                fputc('"', fp2);
                fputc(',', fp2);
                fputs(vars[j].name, fp2);
            }
            fputs(");\n", fp2);
        }
        else if (vars[j].type == 1){
            fprintf(fp2, "    for(int z = 0; z < %d; z++){\n        printf(%c%cd %c, %s[z]);\n    }\n", vars[j].len, '"', '%','"', vars[j].name);

        }
    }
    else if (isdigit(pars[i].name[0])){
        i ++;
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
            if (strcmp(pars[ind].name, "semicolon") == 0){
                ind ++;
            }
            else if (strcmp(pars[ind].type, "Num") == 0){
                ind ++;
                len ++;
            }
            vars[index].len = len;
        }
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
            if (strcmp(pars[find].name, "semicolon") == 0){
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
        puts("ee");
        find --;
        fputs("    ", fp2);
        fputs(vars[index].name, fp2);
        fputs(" = ", fp2);
        find ++;
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
    find -= 3;
    fprintf(fp2, "    %s[", pars[find].name);
    find += 2;
    fprintf(fp2, "%s] = ", pars[find].name);
    find += 3;
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
    fputs(";\n", fp2);
    return find;
}

void increment(FILE *fp2, char val[]){
    fputs("    ", fp2);
    fputs(val, fp2);
    fputs("++;\n", fp2);
}
int structure(FILE *fp2, int ind, char name[]){
    ind ++;
    int i = varind(pars[ind].name);
    puts(pars[ind].name);
    if (1 == vars[i].type){
        fprintf(fp2,"    %s(", name);
        fprintf(fp2, "%s[",pars[ind].name);
        ind += 2;
        fprintf(fp2, "%s]",pars[ind].name);
        ind += 2;
        switch (pars[ind].name[0]){
            case '=':
                fputs(" == ", fp2);
                break;
            case '<':
                fputs(" < ", fp2);
                break;
            case '>':
                fputs(" > ", fp2);
                break;
        }
        ind ++;
        i = varind(pars[ind].name);
        if (1 == vars[i].type){
            fprintf(fp2, "%s[",pars[ind].name);
            ind += 2;
            fprintf(fp2, "%s]",pars[ind].name);
            ind += 2;
        }
        else{
            fputs(pars[ind].name, fp2);
            ind++;
        }
        fputs("){\n", fp2);
    }
    else{
        ind++;
        if(strcmp(pars[ind].type, "Opp") == 0){
            ind --;
            fprintf(fp2,"    %s(", name);
            if ((strcmp(pars[ind].type, "Num") == 0) || (isvar(pars[ind].name))){
                fputs(pars[ind].name,fp2);
            }
            ind ++;
            switch (pars[ind].name[0]){
                case '=' :
                    fputs(" == ", fp2);
                    break;
                case '<' :
                    fputs(" < ", fp2);
                    break;
                case '>' :
                    fputs(" > ", fp2);
                    break;
            }
            ind ++;
            if ((strcmp(pars[ind].type, "Num") == 0) || (isvar(pars[ind].name))){
                fputs(pars[ind].name,fp2);
            }
            fputs("){\n", fp2);
            ind ++;
        }
    }
    return ind;
}

int main(){
    FILE *fp1 = fopen ("Examples/brute_sort.zd", "r");
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
                }
                k ++;
            }
        }
        if(c == '\n'){
            k --;
            if(strcmp(pars[k].name, "Ter") != 0){
                k ++;
                strcpy(pars[k].type, "Ter");
                strcpy(pars[k].name, "Ter");
            }
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
                    i ++;
                }
                else if (strcmp(pars[i].name, "end") == 0){
                    fputs("    }\n", fp2);
                }
                else if (strcmp(pars[i].name, "while") == 0){
                    i = structure(fp2, i, "while");
                }
                break;
            case 'O':
                if (strcmp(pars[i].name, "=") == 0){
                    i --;
                    if (strcmp(pars[i].type, "Str") == 0){
                        if(isvar(pars[i].name) == 0){
                            createvar(fp2, k, i);
                            i ++;
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
                    else if(strcmp(pars[i].name, "rightsquarebracket") == 0){
                        i = assignarray(fp2, i);
                    }
                }
                break;
            case 'S':
                if(isvar(pars[i].name)){
                    i++;
                    if(pars[i].name[0] == '+'){
                        i++;
                        if(pars[i].name[0] == '+'){
                            i -= 2;
                            increment(fp2,pars[i].name);
                        }
                    }
                    else if(pars[i].name[0] == '='){
                        i--;
                        if(isvar(pars[i].name) == 0){
                            createvar(fp2, k, i);
                            i ++;
                            i = assignvar(fp2, k, i);
                            k++;
                        }
                        else{
                            j = varind(pars[i].name);
                            i ++;
                            i = assignvar(fp2, j, i);
                        }
                    }
                }
                break;
        }
    }
    fclose(fp1);
    fputs("    return 0;\n", fp2);
    fputs("}\n", fp2);
    fclose(fp2);
    return 0;
}
