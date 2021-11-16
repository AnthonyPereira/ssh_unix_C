#include "utils.h"

int main(void){
    int sfd;
    if((sfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[mysshd] Socket OK\n");

    int enable = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[mysshd] Setsockopt OK\n");

    int socketsClients[MAX_CLIENTS];
    for(int i=0; i<MAX_CLIENTS; i++){
        socketsClients[i] = 0; 
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons((uint16_t) 1344);

    if(bind(sfd, (struct sockaddr *) &servAddr, sizeof(servAddr))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[mysshd] Bind OK\n");
    
    if(listen(sfd, SOMAXCONN)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    printf("[mysshd] Listen OK\n");

    fd_set socketDescs;
    int nbSockets, i, currentSocket, acceptedSocket;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize;
    
    int authRequest;
    char buffer[SIZETAB];
    memset(buffer, 0, sizeof(char)*SIZETAB);
    char bufferError[SIZETAB];
    memset(bufferError, 0, sizeof(char)*SIZETAB);
    struct passwd *userPasswd;
    struct spwd *userSpwd;
    int authAnswer;

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

        if(select(nbSockets+1, &socketDescs, NULL, NULL, NULL)==-1){
            printf(YELLOW("Error: %s\n"), strerror(errno));
            exit(1);
        }

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
        }

        for(i=0; i<MAX_CLIENTS; i++){
            currentSocket = socketsClients[i];
            if(FD_ISSET(currentSocket, &socketDescs)){
                if(recv(currentSocket, &authRequest, sizeof(int), 0)==0){
                    clientAddrSize = sizeof(clientAddr);
                    if(getsockname(currentSocket, (struct sockaddr *) &clientAddr, &clientAddrSize)==-1){
                        printf(YELLOW("Error: %s\n"), strerror(errno));
                        exit(1);
                    }
                    printf("Déconnexion d'un hôte - IP: %s, Port: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); 
                    close(currentSocket); 
                    socketsClients[i] = 0; 
                }
                else{
                    if(authRequest==SSH_MSG_USERAUTH_REQUEST){
                        authAnswer = 0;
                        if(recv(currentSocket, &buffer, sizeof(buffer), 0)!=-1){
                            printf("user: [%s]\n", buffer);
                            userPasswd = getpwnam(buffer);
                            userSpwd = getspnam(buffer);
                            if((userPasswd == NULL) && (userSpwd == NULL)){
                                printf("Unknown user\n");
                                authAnswer = SSH_MSG_USERAUTH_FAILURE;
                                strcpy(bufferError, "Unknown user");
                            }
                            if(recv(currentSocket, &buffer, sizeof(buffer), 0)!=-1){
                                printf("service name: [%s]\n", buffer);
                                if(strcmp(buffer, "ssh")){
                                    printf("Service name error\n");
                                    authAnswer = SSH_MSG_USERAUTH_FAILURE;
                                    strcpy(bufferError, "Unknown service name");
                                }
                                if(recv(currentSocket, &buffer, sizeof(buffer), 0)!=-1){
                                    printf("method name: [%s]\n", buffer);
                                    if(strcmp(buffer, "password")){
                                        printf("Method name error\n");
                                        authAnswer = SSH_MSG_USERAUTH_FAILURE;
                                        strcpy(bufferError, "Unknown method name");
                                    }
                                    if(recv(currentSocket, &buffer, sizeof(buffer), 0)!=-1){
                                        printf("password: [*****]\n");
                                        if(authAnswer != SSH_MSG_USERAUTH_FAILURE){
                                            if(strcmp(userPasswd->pw_passwd, "x") != 0){
                                                if(strcmp(userPasswd->pw_passwd, crypt(buffer, userPasswd->pw_passwd))){
                                                    printf("Mot de passe incorrect (pw_passwd)\n");
                                                    authAnswer = SSH_MSG_USERAUTH_FAILURE;
                                                    strcpy(bufferError, "Mot de passe incorrect");
                                                }
                                            }
                                            else{
                                                if(strcmp(userSpwd->sp_pwdp, crypt(buffer, userSpwd->sp_pwdp))){
                                                    printf("Mot de passe incorrect (sp_pwdp)\n");
                                                    authAnswer = SSH_MSG_USERAUTH_FAILURE;
                                                    strcpy(bufferError, "Mot de passe incorrect");
                                                }
                                            }
                                        }
                                        if(authAnswer==0) authAnswer = SSH_MSG_USERAUTH_SUCCESS;
                                        if(authAnswer == SSH_MSG_USERAUTH_SUCCESS){
                                            // FORK + LANCER myssh-server
                                            // ...
                                            // ...
                                            // ...
                                            if(send(socketsClients[i], &authAnswer, sizeof(authAnswer), 0)==-1){
                                                printf(YELLOW("Error: %s\n"), strerror(errno));
                                                exit(1);
                                            }
                                        }
                                        else if(authAnswer == SSH_MSG_USERAUTH_FAILURE){
                                            if(send(socketsClients[i], &authAnswer, sizeof(authAnswer), 0)==-1){
                                                printf(YELLOW("Error: %s\n"), strerror(errno));
                                                exit(1);
                                            }
                                            if(send(socketsClients[i], &bufferError, sizeof(bufferError), 0)==-1){
                                                printf(YELLOW("Error: %s\n"), strerror(errno));
                                                exit(1);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } 
    }

    close(sfd);
    exit(0);
}
