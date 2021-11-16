#include "decoupeCommande.h"

int nbCarac(char* cmd){
    int cpt = 0;
    while(*cmd != '\0'){
        cpt++;
        cmd++;
    }
    return cpt;
}

int verifEqual(char* str, char* verif){
    while(*verif != '\0'){
        if(*verif != *str ){
            return 0;
        }
        verif++;
        str++;
    }
    return 1;
}

int nbSeparator(char *cmd, char* separator){
    int cpt = 0;
    if(*cmd == *separator){
        cmd++;
    }
    while(*(cmd+1) != '\0'){
        if(verifEqual(cmd,separator)){
            cpt++;
        }
        cmd++;
    }
    return cpt;
}

int hasAnd(char* cmd){
    while(*cmd != '\0'){
        if(*cmd == '&' && *(cmd+1) == '&'){
            return 1;
        }
        cmd++;
    }
    return 0;
}

int hasOr(char* cmd){
    while(*cmd != '\0'){
        if(*cmd == '|' && *(cmd+1) == '|'){
            return 1;
        }
        cmd++;
    }
    return 0;
}

int hasRedirection(char * cmd){
    int ret = 0;
    while(*cmd != '\0'){
        if(verifEqual(cmd, TUBE) && !verifEqual(cmd+1, TUBE) && !verifEqual(cmd-1, TUBE)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTFINALL)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLEALL)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTFINERR)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLEERR)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTFIN)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLE)){
            ret++;
        }
        else if(verifEqual(cmd, REDIRECTSTDIN)){
            ret++;
        }
        cmd++;
    }
    return ret;
}

char* firstRedirection(char *cmd){
    while(*cmd != '\0'){
        if(verifEqual(cmd, TUBE)){
            return TUBE;
        }
        else if(verifEqual(cmd, REDIRECTFINALL)){
            return REDIRECTFINALL;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLEALL)){
            return REDIRECTSIMPLEALL;
        }
        else if(verifEqual(cmd, REDIRECTFINERR)){
            return REDIRECTFINERR;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLEERR)){
            return REDIRECTSIMPLEERR;
        }
        else if(verifEqual(cmd, REDIRECTFIN)){
            return REDIRECTFIN;
        }
        else if(verifEqual(cmd, REDIRECTSIMPLE)){
            return REDIRECTSIMPLE;
        }
        else if(verifEqual(cmd, REDIRECTSTDIN)){
            return REDIRECTSTDIN;
        }
        cmd++;
    }
    return NULL;
}

char* firstSeparator(char* cmd){
    while(*cmd != '\0'){
        if(verifEqual(cmd, AND)){
            return AND;
        }
        else if(verifEqual(cmd, OR)){
            return OR;
        }
        cmd++;
    }
    return NULL;
}

char* takeBeforeSign(char* cmd, char* sign){
    int nbChar = strlen(cmd);
    char* command = calloc((nbChar+1), sizeof(char));
    //char* command = malloc(sizeof(char)*(nbChar+1));
    //memset(command, 0, sizeof(char)*(nbChar+1));
    int cpt=0, cptSign = 0;
    int fin = 0;
    while(*cmd != '\0' && !fin){
        *(command+cpt) = *cmd;
        cmd++;
        cpt++;
        cptSign=0;
        while(*(cmd+cptSign) == *(sign+cptSign)){
            cptSign++;
        }
        if(*(sign+cptSign) == '\0'){
            fin =1;
        }
    }
    return command;
}

char* takeAfterSign(char* cmd, char* sign){
    int cpt=0, cptCmd=0;
    char *ret = malloc(sizeof(char)*(strlen(cmd)+1));
    while(*cmd != '\0'){
        if(*(sign+cpt) == *cmd){
            cpt++;
        }
        else{
            cpt=0;
        }
        if (*(sign+cpt) == '\0'){
            strcpy(ret, cmd+1);
            return ret;
        }
        cmd++;
        cptCmd++;
    }
    return NULL;
}

char** listCommand(char* cmd, char* cutting, int nbCutting, int nbChar){
    char str[nbChar];
    char** cutByCutting = (char**) malloc(sizeof(char*)*(nbCutting+5));
    int i=0;
    strcpy(str,cmd);
    char* command = strtok(str, cutting);
    cutByCutting[i] = malloc(sizeof(char)*(nbCarac(command)+5));
    strcpy(cutByCutting[i], command);
    i++;
    for(; i<nbCutting; i++){
        command = strtok(NULL, cutting);
        cutByCutting[i] = malloc(sizeof(char)*(nbCarac(command)+5));
        strcpy(cutByCutting[i], command);
    }
    return cutByCutting;
}

/*
int main(){
    char* cmd = "ls ; ls";
    char* separator = ";";
    int nbSepar = nbSeparator(cmd, separator);
    int nbChar= nbCarac(cmd);
    char** list=listCommand(cmd,separator,nbSepar,nbChar);
    for (int i=0;i<nbSepar;i++){
        printf("%s\n", list[i]);
    }
}*/
