#include "utils.h"

char *varEnv[SIZETAB];

void signalCtrlC(){
    for(int i=0; i<SIZETAB; i++){
        free(varEnv[i]);
    }
    exit(0);
}

int main(int argc, char *argv[], char *envp[]){
    (void) argc;
    (void) argv;
    signal(SIGINT, signalCtrlC);

    int i;
    for(i=0; i<SIZETAB; i++){
        if(envp[i]!=NULL){
            varEnv[i] = malloc((strlen(envp[i])+1)*sizeof(char));
            strcpy(varEnv[i], envp[i]);
        }
        else{
            break;
        }
    }
    
    int sfd;
    if((sfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[myssh-server] Socket OK\n");

    int enable = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[myssh-server] Setsockopt OK\n");

    int socketsClients[MAX_CLIENTS];
    for(i=0; i<MAX_CLIENTS; i++){
        socketsClients[i] = 0; 
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons((uint16_t) 1345);

    if(bind(sfd, (struct sockaddr *) &servAddr, sizeof(servAddr))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[myssh-server] Bind OK\n");
    
    if(listen(sfd, SOMAXCONN)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[myssh-server] Listen OK\n");

    fd_set socketDescs;
    int nbSockets, currentSocket, selectRet, acceptedSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize;
    
    char buffer[SIZETAB];
    memset(buffer, 0, sizeof(char)*SIZETAB);
    char bufferSauv[SIZETAB];
    memset(bufferSauv, 0, sizeof(char)*SIZETAB);
    char bufferEnv[SIZETAB];
    memset(bufferEnv, 0, sizeof(char)*SIZETAB);
    char bufferEnvSub[SIZETAB];
    memset(bufferEnvSub, 0, sizeof(char)*SIZETAB);
    char outputCmd[SIZETAB];
    memset(outputCmd, 0, sizeof(char)*SIZETAB);

    ssize_t n;    
    char *tab[SIZETAB] = {"./mysh", "serv", NULL};
    int inputPipes[MAX_CLIENTS][2];
    int outputPipes[MAX_CLIENTS][2];
    for(i=0; i<MAX_CLIENTS; i++){
        inputPipes[i][0] = 0;
        inputPipes[i][1] = 0;
        outputPipes[i][0] = 0;
        outputPipes[i][1] = 0;
    }
    pid_t p;
    int j, lenSubString, trouve, existe;

    for(;;){
        FD_ZERO(&socketDescs);
        FD_SET(sfd, &socketDescs);
        nbSockets = sfd;
        
        for(i=0; i<MAX_CLIENTS; i++){
            currentSocket = socketsClients[i];
            if(currentSocket>0){
                FD_SET(currentSocket, &socketDescs);
            }
            if(currentSocket>nbSockets){
                nbSockets = currentSocket;
            }
        }

        struct timeval selectTimeout = {0, 200000};
        if((selectRet=select(nbSockets+1, &socketDescs, NULL, NULL, &selectTimeout))==-1){
            printf(YELLOW("Error: %s\n"), strerror(errno));
            exit(1);
        }

        if(selectRet){
            if(FD_ISSET(sfd, &socketDescs)){
                clientAddrSize = sizeof(clientAddr);
                if((acceptedSocket=accept(sfd, (struct sockaddr *) &clientAddr, &clientAddrSize))==-1){
                    printf(YELLOW("Error: %s\n"), strerror(errno));
                    exit(1);
                }
                
                printf("Nouvelle connexion - IP: %s, Port: %d, Socket fd: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), acceptedSocket); 
                    
                for(i=0; i<MAX_CLIENTS; i++){
                    if(socketsClients[i] == 0){
                        socketsClients[i] = acceptedSocket;
                        break;
                    }
                }

                if(pipe(inputPipes[i])==-1){
                    printf("Erreur Pipe Input\n");
                }
                if(pipe(outputPipes[i])==-1){
                    printf("Erreur Pipe Output\n");
                }

                if(fcntl(outputPipes[i][0], F_SETFL, O_NONBLOCK) < 0) exit(2);

                p = fork();
                if(p){
                    // Père
                    close(inputPipes[i][0]);
                    close(outputPipes[i][1]);

                    while((n=read(outputPipes[i][0], outputCmd, SIZETAB*sizeof(char)))<=0);

                    outputCmd[n] = '\0';
                    if(send(acceptedSocket, &outputCmd, sizeof(outputCmd), 0)==-1){
                        printf(YELLOW("Error: %s\n"), strerror(errno));
                        exit(1);
                    }
                }
                else{
                    // Fils
                    close(inputPipes[i][1]);
                    close(outputPipes[i][0]);

                    if(dup2(inputPipes[i][0], 0)==-1){
                        printf("Erreur 1\n");
                    }
                    if(dup2(outputPipes[i][1], 1)==-1){
                        printf("Erreur 2\n");
                    }
                    if(dup2(outputPipes[i][1], 2)==-1){
                        printf("Erreur 3\n");
                    }
                    close(inputPipes[i][0]);
                    close(outputPipes[i][1]);

                    execvp("./mysh", tab);
                }
            }
        }

        for(i=0; i<MAX_CLIENTS; i++){
            currentSocket = socketsClients[i];
            //printf("%d %d\n",i,currentSocket);
            //fflush(stdout);
            if(selectRet){
                if(FD_ISSET(currentSocket, &socketDescs)){
                    if((n=recv(currentSocket, &buffer, sizeof(buffer), 0))==0){
                        clientAddrSize = sizeof(clientAddr);
                        if(getsockname(currentSocket, (struct sockaddr *) &clientAddr, &clientAddrSize)==-1){
                            printf(YELLOW("Error: %s\n"), strerror(errno));
                            exit(1);
                        }
                        printf("Déconnexion d'un hôte - IP: %s, Socket: %d\n", inet_ntoa(clientAddr.sin_addr), currentSocket);
                        close(currentSocket);

                        socketsClients[i] = 0;
                        close(inputPipes[i][1]);
                        close(outputPipes[i][0]);
                    }
                    else{
                        buffer[n] = '\0';
                        if(strlen(buffer)>=7 && strncmp("setenv ", buffer, 7)==0){
                            strncpy(bufferEnvSub, buffer+7, SIZETAB-1);
                            bufferEnvSub[strcspn(bufferEnvSub, "\n")] = 0;
                            strcpy(bufferSauv, bufferEnvSub);
                            bufferSauv[strcspn(bufferEnvSub, "=")] = 0;
                            existe = 0;
                            for(j=0; j<SIZETAB; j++){
                                if(varEnv[j]!=NULL && !strncmp(bufferSauv, varEnv[j], strlen(bufferSauv)-1)){
                                    existe = 1;
                                    strcpy(varEnv[j], bufferEnvSub);
                                    strcpy(bufferEnv, "Cette variable d'environnement existait déjà et a donc été modifiée\n");
                                }
                            }
                            if(!existe){
                                trouve = 0;
                                for(j=0; j<SIZETAB; j++){
                                    if(varEnv[j]==NULL){
                                        trouve = 1;
                                        varEnv[j] = malloc((strlen(bufferEnvSub)+1)*sizeof(char));
                                        strcpy(varEnv[j], bufferEnvSub);
                                        strcpy(bufferEnv, "La variable d'environnement a bien été ajoutée\n");
                                        break;
                                    }
                                }
                                if(!trouve) strcpy(bufferEnv, "Vous ne pouvez plus ajouter de variable d'environnement\n");
                            }
                            if(send(currentSocket, &bufferEnv, sizeof(bufferEnv), 0)==-1){
                                printf(YELLOW("Error: %s\n"), strerror(errno));
                                exit(1);
                            }
                            dprintf(inputPipes[i][1], " ");
                        }
                        else if(strlen(buffer)>=10 && strncmp("unsetenv $", buffer, 10)==0){
                            strcpy(bufferEnvSub, buffer+10);
                            lenSubString = strlen(bufferEnvSub);
                            trouve = 0;
                            for(j=0; j<SIZETAB; j++){
                                if(varEnv[j]!=NULL && !strncmp(bufferEnvSub, varEnv[j], lenSubString-1)){
                                    trouve = 1;
                                    free(varEnv[j]);
                                    varEnv[j] = NULL;
                                    strcpy(bufferEnv, "La variable d'environnement a bien été supprimée\n");
                                    break;
                                }
                            }
                            if(!trouve) strcpy(bufferEnv, "Cette variable d'environnement n'existe pas\n");
                            if(send(currentSocket, &bufferEnv, sizeof(bufferEnv), 0)==-1){
                                printf(YELLOW("Error: %s\n"), strerror(errno));
                                exit(1);
                            }
                            dprintf(inputPipes[i][1], " ");
                        }
                        else if(strlen(buffer)>=6 && strncmp("echo $", buffer, 6)==0){
                            strcpy(bufferEnvSub, buffer+6);
                            lenSubString = strlen(bufferEnvSub);
                            trouve = 0;
                            for(j=0; j<SIZETAB; j++){
                                if(varEnv[j]!=NULL && !strncmp(bufferEnvSub, varEnv[j], lenSubString-1)){
                                    trouve = 1;
                                    memset(bufferEnv, 0, sizeof(char)*SIZETAB);
                                    strncpy(bufferEnv, varEnv[j]+lenSubString, SIZETAB-5);
                                    strcat(bufferEnv, "\n");
                                    if(send(currentSocket, &bufferEnv, sizeof(bufferEnv), 0)==-1){
                                        printf(YELLOW("Error: %s\n"), strerror(errno));
                                        exit(1);
                                    }
                                    dprintf(inputPipes[i][1], " ");
                                    break;
                                }
                            }
                            if(!trouve) dprintf(inputPipes[i][1], "%s", buffer);
                        }
                        else{
                            dprintf(inputPipes[i][1], "%s", buffer);
                        }
                        /*
                        for(j=0; j<SIZETAB; j++){
                            if(varEnv[j]!=NULL){
                                printf("%s\n",varEnv[j]);
                            }
                        }
                        */
                    }
                }
            }
            if(currentSocket!=0){
                if((n=read(outputPipes[i][0], outputCmd, SIZETAB*sizeof(char)))>0){
                    outputCmd[n] = '\0';
                    //printf("[%s]\n",outputCmd);
                    //fflush(stdout);
                    if(send(currentSocket, &outputCmd, sizeof(outputCmd), 0)==-1){
                        printf(YELLOW("Error: %s\n"), strerror(errno));
                        exit(1);
                    }
                }
            }
        }
    }

    close(sfd);
    for(i=0; i<SIZETAB; i++){
        free(varEnv[i]);
    }
    free(varEnv);
    exit(0);
}
