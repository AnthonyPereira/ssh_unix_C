#include "utils.h"

int main(int argc, char *argv[]){
    if(argc<2){
        printf(YELLOW("Erreur : Veuillez spécifier l'utilisateur et l'adresse IP (ou : un alias).\nUtilisation : myssh utilisateur@adresseIP [-c commande] (ou : myssh alias [-c commande])\n"));
        exit(1);
    }
    if(argc==3 || (argc>2 && strcmp(argv[2], "-c"))){
        printf(YELLOW("Erreur : Pour préciser une commande à exécuter, utiliser l'option -c suivie de la commande à exécuter.\nUtilisation : myssh utilisateur@adresseIP [-c commande] (ou : myssh alias [-c commande])\n"));
        exit(1);
    }

    char user[SIZETAB];
    memset(user, 0, sizeof(char)*SIZETAB);
    int posUser = 0;

    char hostname[SIZETAB];
    memset(hostname, 0, sizeof(char)*SIZETAB);
    int posHostname = 0;

    int i;

    if(strchr(argv[1], '@') != NULL){
        // Utilisateur@adresseIP
        for(i=0; argv[1][i]!='@'; i++){
            user[posUser++] = argv[1][i];
        }
        for(i++; argv[1][i]!='\0'; i++){
            hostname[posHostname++] = argv[1][i];
        }
    }
    else{
        // Alias
        FILE *f = fopen(".myssh/config", "r");
        if(f==NULL){
            printf("Cannot open file /.myssh/config\n");
            exit(1);
        }

        char *line = NULL;
        size_t len = 0;

        char *aliasSearch = malloc(10+strlen(argv[1]));
        strcpy(aliasSearch, "Host ");
        strcat(aliasSearch, argv[1]);

        int trouve = 0;
        while(getline(&line, &len, f) != -1){
            line[strlen(line)-1] = '\0';
            if(!strcmp(line, aliasSearch)){
                trouve = 1;
                break;
            }
        }

        if(trouve){
            if(getline(&line, &len, f) != -1){
                line[strlen(line)-1] = '\0';
                i=0;
                while(line[i]==' ') i++;
                for(i+=9; line[i]!='\0'; i++){
                    hostname[posHostname++] = line[i];
                }
            }
            if(getline(&line, &len, f) != -1){
                line[strlen(line)-1] = '\0';
                i=0;
                while(line[i]==' ') i++;
                for(i+=5; line[i]!='\0'; i++){
                    user[posUser++] = line[i];
                }
            }
        }
        else{
            printf(YELLOW("Erreur : Alias inconnu. Vérifiez le fichier .myssh/config\nUtilisation : myssh alias (ou : myssh utilisateur@adresseIP)\n"));
            free(line);
            free(aliasSearch);
            fclose(f);
            exit(1);
        }

        free(line);
        free(aliasSearch);
        fclose(f);
    }
    printf("User: %s\n", user);
    printf("Hostname: %s\n", hostname);

    int sfd;
    if((sfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons((uint16_t) 1344);
    if(!inet_aton(hostname, &clientAddr.sin_addr)){
        printf(YELLOW("Error: invalid address (inet_aton)\n"));
        exit(1);
    }

    if(connect(sfd, (struct sockaddr*) &clientAddr, sizeof(clientAddr))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    
    // Mot de passe
    char password[SIZETAB];
    memset(password, 0, sizeof(char)*SIZETAB);

    printf("Mot de passe: ");
    fflush(stdout);

    struct termios oflags, nflags;
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    // Désactiver l'affichage stdin
    if(tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0){
        perror("tcsetattr");
        exit(1);
    }
    
    // Demande du mot de passe
    if(read(0, password, SIZETAB*sizeof(char)) == -1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    char *p;
    if((p = strchr(password, '\n')) != NULL) *p = '\0';

    // Réactiver l'affichage stdin
    if(tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0){
        perror("tcsetattr");
        return -1;
    }

    //printf("(%s)\n", password);
    
    int authRequest = SSH_MSG_USERAUTH_REQUEST;
    char buffer[SIZETAB];
    memset(buffer, 0, sizeof(char)*SIZETAB);
    int authAnswer = 0;

    if(send(sfd, &authRequest, sizeof(authRequest), 0)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    if(send(sfd, &user, sizeof(user), 0)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    strcpy(buffer, "ssh");
    if(send(sfd, &buffer, sizeof(buffer), 0)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    strcpy(buffer, "password");
    if(send(sfd, &buffer, sizeof(buffer), 0)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    if(send(sfd, &password, sizeof(password), 0)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }

    if(recv(sfd, &authAnswer, sizeof(int), 0)!=-1){
        if(authAnswer==SSH_MSG_USERAUTH_FAILURE){
            if(recv(sfd, &buffer, sizeof(buffer), 0)!=-1){
                printf("%s\n", buffer);
            }
            exit(1);
        }
    }

    if(close(sfd)==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }

    printf("Authentification effectuée\n");

    if((sfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }
    
    clientAddr.sin_port = htons((uint16_t) 1345);

    if(connect(sfd, (struct sockaddr*) &clientAddr, sizeof(clientAddr))==-1){
        printf(YELLOW("Error: %s\n"), strerror(errno));
        exit(1);
    }

    ssize_t n;

    if(argc>2){
        char execCmd[SIZETAB];
        memset(execCmd, 0, sizeof(char)*SIZETAB);
        for(i=3; i<argc; i++){
            strcat(execCmd, argv[i]);
            strcat(execCmd, " ");
        }
        strcat(execCmd, "; exit");
        //printf("[%s]\n", execCmd);
        if(send(sfd, execCmd, strlen(execCmd), 0)==-1){
            printf(YELLOW("Error: %s\n"), strerror(errno));
            exit(1);
        }
        if((n=recv(sfd, buffer, SIZETAB, 0))>0){
            buffer[n] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }
        if((n=recv(sfd, buffer, SIZETAB, 0))>0){
            buffer[n] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }
        close(sfd);
        exit(0);
    }

    if((n=recv(sfd, buffer, SIZETAB, 0))==-1){
        printf(YELLOW("Error: failed reading (recv)\n"));
        exit(1);
    }
    buffer[n] = '\0';
    printf(GREEN("%s@%s"), user, hostname);
    printf(":%s", buffer);
    fflush(stdout);

    if(fcntl(0, F_SETFL, O_NONBLOCK) < 0) exit(2);
    if(fcntl(sfd, F_SETFL, O_NONBLOCK) < 0) exit(2);

    for(;;){
        if((n=read(0, buffer, SIZETAB*sizeof(char)))>0){
            buffer[n] = '\0';
            if(send(sfd, buffer, strlen(buffer), 0)==-1){
                printf(YELLOW("Error: %s\n"), strerror(errno));
                exit(1);
            }
            if(strstr(buffer, "exit")!=NULL){
                break;
            }
        }

        if((n=recv(sfd, buffer, SIZETAB, 0))>0){
            buffer[n] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }
    }

    close(sfd);
    exit(0);
}
