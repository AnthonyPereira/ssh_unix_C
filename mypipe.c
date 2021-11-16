#include "mypipe.h"

void createTabArgs(char** tab[], int taille){
    // Initialisation de la matrice
    for(int i=0;i<taille;i++){
        tab[i]=calloc(sizeof(char*),SIZETAB);
        for(int j=0;j<SIZETAB;j++){
            tab[i][j]=NULL;
        }
    }
}

void resetTabArg(char** tab[], int taille){
    for(int i=0;i<taille;i++){
        tab[i]=calloc(sizeof(char*),SIZETAB);
        for(int j=0;j<taille;j++){
            tab[i][j]=NULL;
        }
    }
}

int pipeSuivant(char **cmdargs[], int i, int max){
    if(i<max-1){
        if((*cmdargs[i+1])!=NULL)
            return !strcmp(*cmdargs[i+1],"|");
        return 0;
    }
    return 0;
}

void closePipeNotUsed(int (*tabPipe)[2], int actuel){
    for(int i=0;i<actuel;i++){
        close(tabPipe[i][1]);
        close(tabPipe[i][0]);
    }
}

void closeAllPipe(int (*tabPipe)[2], int maxPipe){
    for(int i=0;i<maxPipe;i++){
        close(tabPipe[i][1]);
        close(tabPipe[i][0]);
    }
}

char* getCmd(char** cmdargs){
    char* buf=calloc(sizeof(char),SIZETAB);
    while(*cmdargs!=NULL){
        strcat(buf,*cmdargs);
        strcat(buf," ");
        cmdargs++;
    }
    return buf;
}

int doPipe(char *inputCmd){
    int nbPipe;
    int status;
    int tailleTabCmd;
    int nbCarCmd;
    int nbCarArg;
    char** allCommand;
    char** allArgs;
    char *s;
    nbPipe = nbSeparator(inputCmd, TUBE);

    tailleTabCmd = nbPipe*2+1;
    char** cmdargs[tailleTabCmd];
    createTabArgs(cmdargs,(tailleTabCmd+1));

    for(s=inputCmd; isspace(*s); s++);
    nbCarCmd = nbCarac(inputCmd);
    allCommand = listCommand(inputCmd,TUBE,(nbPipe+1),nbCarCmd);
    for(int cpt= 0, i = 0; cpt < tailleTabCmd; i++, cpt++){
        int nbCmd;
        s=allCommand[i];
        nbCmd = nbSeparator(s, " ")+1;
        nbCarArg = nbCarac(s);
        allArgs = listCommand(s," ",nbCmd, nbCarArg);
        for(int j = 0; j < (nbCmd+1);j++){
            cmdargs[cpt][j] = allArgs[j];
        }
        cpt++;
        if(cpt < tailleTabCmd){
            cmdargs[cpt][0] = TUBE;
        }
    }

    int processCurr=0;
    int curr=0;
    char* cmd=calloc(sizeof(char),SIZETAB);
    char* res;
    
    nbPipe++;

    // Création du tableau de pipes et de processus
    int (*tabPipe)[2] = calloc(sizeof(int*),tailleTabCmd); 
    pid_t* tabProcess = calloc(sizeof(pid_t),tailleTabCmd);

    for(int i=0;i<nbPipe;i++){
        if(pipe(tabPipe[i])==ERR){
            perror("Error Pipe x");
            exit(1);
        }
    }
    while(curr<tailleTabCmd && ((**cmdargs)!=NULL)){
        if(strcmp("|",**cmdargs)){
            if(processCurr==0){
                if((tabProcess[processCurr]=fork())==ERR){
                    perror("Error Fork 1");
                    exit(1);
                }
                processCurr++;
                if(!tabProcess[processCurr-1]){
                    close(1); // Fermeture sortie standard
                    if(dup(tabPipe[processCurr-1][1])==ERR){ // Redirection de la sortie standard vers le tube en écriture
                        perror("Error dup");
                        exit(1);
                    } 
                    close(tabPipe[processCurr-1][1]); // Fermeture du descripteur initial du tube
                    close(tabPipe[processCurr-1][0]); // Lecture non utilisée
                    //execlp( "ls", "ls", 0 );
                    execvp(**cmdargs, *cmdargs);
                    perror("Error Exec");
                    exit(EXIT_FAILURE);
                }
                else{
                    wait(&status);
                    res=getCmd(cmdargs[curr]);
                    strcat(cmd,res);
                    free(res);

                    if((tabProcess[processCurr]=fork())==ERR){
                        perror("Error Fork 1");
                        exit(1);
                    }
                    processCurr++;
                }
            }
            else{
                if(!tabProcess[processCurr-1]){
                    close(0);

                    closePipeNotUsed(tabPipe,processCurr-2);

                    if(dup(tabPipe[processCurr-2][0])==ERR){
                        perror("Error Dup 1");
                        exit(1);
                    }

                    close(tabPipe[processCurr-2][1]);
                    close(tabPipe[processCurr-2][0]);
                    

                    close(1);
                    if(dup(tabPipe[processCurr-1][1])==ERR){
                        perror("Error Dup 1");
                        exit(1);
                    }
                    //execlp( "sort", "sort","-r", 0); // recouvrement du processus

                    close(tabPipe[processCurr-1][0]);
                    close(tabPipe[processCurr-1][1]);

                    execvp(**cmdargs,*cmdargs);
                    
                    perror("Exec Error");
                    exit(127);
                }
                // Traiter père si pipe suivant
                else{
                    closePipeNotUsed(tabPipe,processCurr-1);
                    wait(&status);
                    res=getCmd(cmdargs[curr]);
                    strcat(cmd,res);
                    if(pipeSuivant(cmdargs,curr,tailleTabCmd)){
                        if((tabProcess[processCurr]=fork())==ERR){
                            perror("Error Fork 1");
                            exit(1);
                        }
                        processCurr++;
                    }
                }
            }
        }
        else{
            if(tabProcess[processCurr-1]){
                res=getCmd(cmdargs[curr]);
                strcat(cmd,res);
            }
        }
        *cmdargs=cmdargs[curr+1];
        curr++;
    }
    if(tabProcess[processCurr-1]){
        closePipeNotUsed(tabPipe,processCurr-1);
        wait(&status);
        // close(pipe1[0]);
        // close(pipe1[1]);
        char* buf=calloc(sizeof(char),SIZETAB);
        if(read(tabPipe[processCurr-1][0],buf,SIZETAB)==ERR){
            perror("Error Read");
            exit(1);
        }
        printf("%s",buf);
        close(tabPipe[processCurr-1][0]);
        close(tabPipe[processCurr-1][1]);
        // printf("%s\n",cmd);
    }
    free(cmd);
    free(tabPipe);
    free(tabProcess);
    free(allCommand);

    resetTabArg(cmdargs,tailleTabCmd);

    return status;
}
