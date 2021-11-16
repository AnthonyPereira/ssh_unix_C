#include "utils.h"

#include "decoupeCommande.h"
#include "mypipe.h"
#include "variableLocal.h"

void signalCtrlC(){ // Gérer lorsqu'une commande est en cours d'execution (voir MYSH 2.b.)
    char absPathname[SIZETAB], inputChoice[SIZETAB];

    do{
        memset(inputChoice, 0, SIZETAB*sizeof(char));
        printf(RED_BOLD("\nVoulez-vous vraiment quitter et tuer tous les processus en tâches de fond ? ['Y' pour oui / 'n' sinon] : "));
        fflush(stdout);
        ERRORCHECK(read(0, inputChoice, SIZETAB*sizeof(char)), -1);
    }while(strlen(inputChoice)!=2 || (inputChoice[0] != 'Y' && inputChoice[0] != 'n'));

    if(inputChoice[0]=='Y'){
        exit(0);
    }

    ERRORCHECK(getcwd(absPathname, SIZETAB*sizeof(char)), NULL);
    printf(BLUE_BOLD("%s> "), absPathname);
    fflush(stdout);
}

int checkSeparator(char** resteCmd, int* nbCommandInCmd, char** first, char** cmd, int* cmdSuccess){
    int cmdReady = 0;
    char *ret;
    while(!cmdReady){
        if(hasAnd(*resteCmd) || hasOr(*resteCmd)){
            if((*first == NULL) || (strcmp(*first, AND)==0 && *cmdSuccess != 1) || (strcmp(*first, OR)==0 && *cmdSuccess == 1)) {
                *nbCommandInCmd = nbSeparator(*resteCmd, AND)*2 + nbSeparator(*resteCmd, OR)*2;
                *first = firstSeparator(*resteCmd);
                *cmd = takeBeforeSign(*resteCmd, *first);
                ret = takeAfterSign(*resteCmd, *first);
                strcpy(*resteCmd, ret);
                free(ret);
                cmdReady = 1;
            }
            else{
                (*nbCommandInCmd)--;
                *first = firstSeparator(*resteCmd);
                ret = takeAfterSign(*resteCmd, *first);
                strcpy(*resteCmd, ret);
                free(ret);
            }
        }
        else {
            if((*first == NULL) || (*first != NULL && ((strcmp(*first, AND)==0 && *cmdSuccess != 1) || (strcmp(*first, OR)==0 && *cmdSuccess == 1)))){
                *cmd = *resteCmd;
            }
            else{
                return 1;
            }
            cmdReady = 1;
        }
    }
    return 0;
}

int checkRedirection(char** resteCmd, char** first, char** cmd){
    *first = firstRedirection(*resteCmd);
    *cmd = takeBeforeSign(*resteCmd,*first);
    *resteCmd = takeAfterSign(*resteCmd,*first);
    return 1;
}

int main(int argc, char *argv[]){
    Variable *chaineVar=NULL;
    char absPathname[SIZETAB], inputCmd[SIZETAB], saveProc[SIZETAB];
    char *tabCmd[SIZETAB], *s;
    pid_t p, pReDirect=getpid(), pReDirect2;
    int i, cpt, totalCmdArgs, status, stateStatus=-1, stop=0;
    int nbCommand, nbCommandInCmd, nbCarCmd;
    char** allCommand;
    char *cmd, *resteCmd, *fic, *redirect;
    char *cmdCopy;
    char* delimiterCommand = ";";
    char* first = NULL;
    int tubeRedirectStdout[2], tubeRedirectStderr[2];
    int tubeOn = 0;
    int nbRedirection, faireCmd, estFils=0;
    int mode=0;

    signal(SIGINT, signalCtrlC);
    if(argc>1 && !strcmp(argv[1],"serv")) mode=1;

    for(;;){
        ERRORCHECK(getcwd(absPathname, SIZETAB*sizeof(char)), NULL);
        printf(BLUE_BOLD("%s> "), absPathname);
        fflush(stdout);
        memset(inputCmd, 0, SIZETAB*sizeof(char));
        ERRORCHECK(read(0, inputCmd, SIZETAB*sizeof(char)), -1);
        //inputCmd[strcspn(inputCmd,"\n")] = '\0';

        nbCommand = nbSeparator(inputCmd, delimiterCommand)+1;
        nbCarCmd = nbCarac(inputCmd);
        allCommand = listCommand(inputCmd, delimiterCommand, nbCommand, nbCarCmd);
        resteCmd = malloc(sizeof(char)*(SIZETAB));

        // Gestion des expressions régulières (; && ||)
        for(i=0; i<nbCommand; i++){
            nbCommandInCmd = 1;
            first = NULL;
            strcpy(resteCmd, allCommand[i]);
            free(allCommand[i]);
            while(nbCommandInCmd){
                cmdCopy = NULL;
                faireCmd = 1;
                stop = checkSeparator(&resteCmd, &nbCommandInCmd, &first, &cmd, &stateStatus);
                if(cmdCopy == NULL) cmdCopy = cmd;
                cpt = 0;
                while(*(cmd+cpt) != '\0'){
                    if(*(cmd+cpt) == '\n'){
                        *(cmd+cpt) = ' ';
                    }
                    else{
                        cpt++;
                    }
                }
                nbRedirection = hasRedirection(resteCmd);
                if(nbRedirection){
                    redirect = firstRedirection(cmd);
                    if(nbSeparator(cmd, TUBE)){
                        stateStatus = 0;
                        faireCmd = 0;
                        strcpy(saveProc, cmd);
                        status = doPipe(cmd);
                    }
                    else{
                        fic = takeAfterSign(cmd, redirect);
                        cmd = takeBeforeSign(cmd, redirect);
                        fic = enleveCarac(fic, ' ');
                        int prendreStdin = (strcmp(redirect, REDIRECTFIN) == 0 || strcmp(redirect, REDIRECTSIMPLE) == 0 || strcmp(redirect, REDIRECTFINALL) == 0 || strcmp(redirect, REDIRECTSIMPLEALL) == 0);
                        int prendreStderr = (strcmp(redirect, REDIRECTFINERR) == 0 || strcmp(redirect, REDIRECTSIMPLEALL) == 0 || strcmp(redirect, REDIRECTSIMPLEERR) == 0 || strcmp(redirect, REDIRECTFINALL) == 0);
                        if(prendreStdin){
                            if(pipe(tubeRedirectStdout)==ERR){
                                perror("Error Pipe ");
                                exit(1);
                            }
                        }
                        if(prendreStderr){
                            if(pipe(tubeRedirectStderr)==ERR){
                                perror("Error Pipe ");
                                exit(1);
                            }
                        }
                        if((pReDirect=fork()) == ERR){
                            perror("probleme fork");
                            exit(1);
                        }
                        if(pReDirect){
                            wait(&status);
                            if((pReDirect2 = fork()) == ERR){
                                perror("probleme fork");
                                exit(1);
                            }
                            if(pReDirect2){
                                //pere
                                free(fic);
                                if(prendreStdin){
                                    close(tubeRedirectStdout[0]);
                                    close(tubeRedirectStdout[1]);
                                }
                                if(prendreStderr){
                                    close(tubeRedirectStderr[0]);
                                    close(tubeRedirectStderr[1]);
                                }
                                wait(&status);
                                faireCmd = 0;
                            }
                            else{
                                //fils 2
                                close(0);
                                if(prendreStdin){
                                    close(tubeRedirectStdout[1]);
                                    if(dup(tubeRedirectStdout[0]) == ERR){
                                        perror("probleme dup");
                                        exit(1);
                                    }
                                    fcntl(tubeRedirectStdout[0], F_GETFD);
                                }
                                if(prendreStderr){
                                    close(tubeRedirectStderr[1]);
                                    if(dup(tubeRedirectStderr[0]) == ERR){
                                        perror("probleme dup");
                                        exit(1);
                                    }
                                    fcntl(tubeRedirectStderr[0], F_GETFD);
                                }
                                
                                FILE* ficOpen;
                                if(strcmp(redirect, REDIRECTSIMPLE) == 0 || strcmp(redirect, REDIRECTSIMPLEALL) == 0 || strcmp(redirect, REDIRECTSIMPLEERR) == 0){
                                    ficOpen = fopen(fic,"w");
                                }
                                else{
                                    ficOpen = fopen(fic,"a");
                                }
                                char c;
                                if(prendreStdin){
                                    while(read(tubeRedirectStdout[0],&c,1)){
                                        fputc(c, ficOpen);
                                    }
                                }
                                if(prendreStderr){
                                    while(read(tubeRedirectStderr[0],&c,1)){
                                        fputc(c, ficOpen);
                                    }
                                }
                                free(fic);
                                fclose(ficOpen);
                                exit(EXIT_SUCCESS);
                            }
                        
                        }
                        else{
                            //fils 1
                            if(prendreStdin){    
                                close(1);
                                close(tubeRedirectStdout[0]);
                                if(dup(tubeRedirectStdout[1])==ERR){
                                    perror("probleme dup");
                                    exit(1);
                                }
                            }
                            if(prendreStderr){
                                close(2);
                                close(tubeRedirectStderr[0]);
                                if(dup(tubeRedirectStderr[1])==ERR){
                                    perror("probleme dup");
                                    exit(1);
                                }
                            }
                            estFils = 1;
                            free(fic);
                        }                        
                    }
                    
                }
                if(stop){
                    stop = 0;
                    break;
                }
                if(verifEqual(cmd, AJOUTVAR)){
                    strcpy(saveProc, cmd);
                    stateStatus = 0;
                    cmd += strlen(AJOUTVAR);
                    
                    if(haveEqualSign(cmd)){
                        chaineVar = siEqual(chaineVar, cmd);
                    }
                    else{
                        chaineVar = ajoutVarSansInit(chaineVar, cmd);
                    }
                    faireCmd = 0;
                    status = 0;
                }
                else if(verifEqual(cmd, SUPPRVAR)){
                    strcpy(saveProc, cmd);
                    stateStatus = 0;
                    cmd += strlen(SUPPRVAR);
                    cmd = enleveCarac(cmd, ' ');
                    chaineVar = supprVar(chaineVar,cmd);
                    faireCmd = 0;
                    status = 0;
                }
                else if(verifEqual(cmd, AFFICHEVAR)){
                    strcpy(saveProc, cmd);
                    stateStatus = 0;
                    cmd += strlen(AFFICHEVAR);
                    char* echo = transformEchoToStr(chaineVar, cmd);
                    printf("%s\n", echo);
                    if(strcmp(echo, "Une variable n'est pas reconnue")){
                        status = 1;
                        free(echo);
                    }
                    else{
                        status = 0;
                    }
                    faireCmd=0;
                }

                if(!faireCmd){
                    nbCommandInCmd--;
                }

                if(faireCmd){
                    nbCommandInCmd--;
                    // printf("%s\n", cmd);
                    // Découpage de la commande dans un tableau
                    if(!tubeOn){
                        for(s=cmd; isspace(*s); s++);
                        for(totalCmdArgs=0; *s; totalCmdArgs++){
                            tabCmd[totalCmdArgs]=s;
                            while(!isspace(*s)) s++;
                            *s++='\0';
                            while(isspace(*s)) s++;
                        }
                    }
                    tabCmd[totalCmdArgs]=NULL;
                    if(totalCmdArgs>0){
                        if(!strcmp(tabCmd[0],"cd")){
                            stateStatus=0;
                            if(totalCmdArgs==1){
                                status = chdir(getenv("HOME"));
                            }
                            else if(totalCmdArgs==2){
                                status = chdir(tabCmd[1]);
                                if(status==-1){
                                    printf(YELLOW("Erreur : Répertoire inconnu.\nUtilisation : cd <répertoire>\n"));
                                }
                                else{
                                    stateStatus=0;
                                }
                            }
                            else{
                                printf(YELLOW("Erreur : Trop d'arguments.\nUtilisation : cd <répertoire>\n"));
                                status=1;
                            }
                        }
                        else if(!strcmp(tabCmd[0],"exit")){
                            stateStatus=0;
                            if(totalCmdArgs==1){
                                supprAllVar(chaineVar, 0);
                                free(allCommand);
                                free(resteCmd);
                                if(cmdCopy!=NULL && cmdCopy!=resteCmd) free(cmdCopy);
                                return 0;
                            }
                            else{
                                printf(YELLOW("Erreur : Trop d'arguments.\nUtilisation : exit\n"));
                                status=1;
                            }
                        }
                        else if(!strcmp(tabCmd[0],"status")){
                            if(totalCmdArgs==1){
                                if(stateStatus==-1){
                                    printf("Aucun processus n'a été exécuté jusqu'à présent.\n");
                                }
                                else if(stateStatus==0){
                                    printf("%s terminé avec comme code de retour %i\n", saveProc, status);
                                }
                                else if(stateStatus==1){
                                    printf("%s terminé anormalement.\n", saveProc);
                                }
                                stateStatus=0;
                                status=0;
                            }
                            else{
                                printf(YELLOW("Erreur : Trop d'arguments.\nUtilisation : status\n"));
                                stateStatus=0;
                                status=1;
                            }
                        }
                        else if(!strcmp(tabCmd[0],"myjobs")){
                            stateStatus=0;
                            if(totalCmdArgs==1){
                                printf("Liste des processus en arrière plan :\n");
                                status=0;
                            }
                            else{
                                printf(YELLOW("Erreur : Trop d'arguments.\nUtilisation : myjobs\n"));
                                status=1;
                            }
                        }
                        else{
                            ERRORCHECK((p=fork()), -1);
                            if(p){ // Père
                                wait(&status);
                                if(WIFEXITED(status)){ // WIFEXITED
                                    stateStatus=0;
                                    status = WEXITSTATUS(status);
                                }
                                else{ // ABNORMAL EXIT
                                    stateStatus=1;
                                }
                            }
                            else{ // Fils
                                ERRORCHECK(execvp(tabCmd[0], tabCmd), -1);
                                exit(EXIT_FAILURE);
                            }
                        }
                        strcpy(saveProc, tabCmd[0]);
                    }
                    if(estFils == 1){
                        exit(stateStatus);
                    }
                }
                //if(cmdCopy!=NULL && cmdCopy!=resteCmd) free(cmdCopy);
            }
        }
        free(allCommand);
        if(mode){
            printf("Processus distant terminé avec le code [%d]\n", status);
            fflush(stdout);
        }
        free(resteCmd);
    }

    return 0;
}
