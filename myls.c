#include "utils.h"

#include <grp.h>

void readDirectory(char* startDir, int optFicCaches, int optExploRecursive){
    DIR *ptD;
    struct dirent *currentDir;
    struct stat fileStats;
    char *fullPath = (char*) malloc(SIZETAB*sizeof(char));
    int lenFullPath = SIZETAB;
    int lenNewPath;
    struct passwd *strtOwner;
    struct group *strtGroup;
    char timeString[50];
    char linkPath[SIZETAB];
    int linkPathLen;

    ptD = opendir(startDir);
    if(ptD){
        // Répertoire
        printf("%s:\n",startDir);
        while((currentDir=readdir(ptD)) != NULL){
            if(optFicCaches || currentDir->d_name[0]!='.'){
                // Chemin fichier
                lenNewPath = strlen(startDir)+strlen(currentDir->d_name)+5;
                if(lenNewPath>lenFullPath){
                    fullPath = (char*) realloc(fullPath, lenNewPath*sizeof(char));
                    lenFullPath = lenNewPath;
                }
                sprintf(fullPath, "%s/%s", startDir, currentDir->d_name);
                if(stat(fullPath, &fileStats)!=0){
                    printf(YELLOW("Error: %s\n"), strerror(errno));
                    continue;
                }
                // Type fichier
                switch(currentDir->d_type){
                    case DT_REG: printf("-"); break; // Fichier normal
                    case DT_DIR: printf("d"); break; // Répertoire
                    case DT_LNK: printf("l"); break; // Lien symbolique
                    case DT_FIFO: printf("p"); break; // Tube nommé
                    case DT_BLK: printf("b"); break; // Périphérique de blocs
                    case DT_CHR: printf("c"); break; // Périphérique de caractères
                    case DT_SOCK: printf("s"); break; // Socket
                    default: printf("?"); break; // Type inconnu (case DT_UNKNOWN)
                }
                // Droits
                printf((fileStats.st_mode & S_IRUSR) ? "r" : "-");
                printf((fileStats.st_mode & S_IWUSR) ? "w" : "-");
                printf((fileStats.st_mode & S_IXUSR) ? "x" : "-");
                printf((fileStats.st_mode & S_IRGRP) ? "r" : "-");
                printf((fileStats.st_mode & S_IWGRP) ? "w" : "-");
                printf((fileStats.st_mode & S_IXGRP) ? "x" : "-");
                printf((fileStats.st_mode & S_IROTH) ? "r" : "-");
                printf((fileStats.st_mode & S_IWOTH) ? "w" : "-");
                printf((fileStats.st_mode & S_IXOTH) ? "x" : "-");
                // Nombre de liaisons
                printf(" %li", fileStats.st_nlink);
                // Propriétaire
                if((strtOwner=getpwuid(fileStats.st_uid))!=NULL){
                    printf(" %s", strtOwner->pw_name);
                }
                else{
                    printf(" 'Unknown user'");
                }
                // Groupe
                if((strtGroup=getgrgid(fileStats.st_gid))!=NULL){
                    printf(" %s", strtGroup->gr_name);
                }
                else{
                    printf(" 'Unknown group'");
                }
                // Taille en octets
                printf(" %li", fileStats.st_size);
                // Date et heure de la dernière modification
                if(strftime(timeString, 50, "%b %d %Y %H:%M", localtime(&fileStats.st_mtime))!=0){
                    printf(" %s", timeString);
                }
                else{
                    printf(" 'Unknown last modification date'");
                }
                // Nom du fichier
                switch(currentDir->d_type){
                    case DT_REG: printf(GREEN_BOLD(" %s\n"), currentDir->d_name); break; // Fichier normal
                    case DT_DIR: printf(BLUE_BOLD(" %s\n"), currentDir->d_name); break; // Répertoire
                    case DT_LNK: // Lien symbolique
                        if((linkPathLen=readlink(fullPath, linkPath, sizeof(linkPath)-1))!=-1){
                            linkPath[linkPathLen]='\0';
                            printf(CYAN_BOLD(" %s -> %s\n"), currentDir->d_name, linkPath);
                        }
                        else{
                            printf(YELLOW("Error: %s\n"), strerror(errno));
                        }
                        break;
                    case DT_FIFO: printf(YELLOW_BOLD(" %s\n"), currentDir->d_name); break; // Tube nommé
                    case DT_BLK: printf(YELLOW_BOLD(" %s\n"), currentDir->d_name); break; // Périphérique de blocs
                    case DT_CHR: printf(YELLOW_BOLD(" %s\n"), currentDir->d_name); break; // Périphérique de caractères
                    case DT_SOCK: printf(MAGENTA_BOLD(" %s\n"), currentDir->d_name); break; // Socket
                    case DT_UNKNOWN: printf(" %s\n", currentDir->d_name); break; // Type inconnu
                }
            }
        }
        closedir(ptD);
        printf("\n");
        
        if(optExploRecursive){
            ptD = opendir(startDir);
            while((currentDir=readdir(ptD)) != NULL){
                if(optFicCaches || currentDir->d_name[0]!='.'){
                    if(optExploRecursive && currentDir->d_type==DT_DIR && strcmp(currentDir->d_name,".")!=0 && strcmp(currentDir->d_name,"..")!=0){
                        lenNewPath = strlen(startDir)+strlen(currentDir->d_name)+5;
                        if(lenNewPath>lenFullPath){
                            fullPath = (char*) realloc(fullPath, lenNewPath*sizeof(char));
                            lenFullPath = lenNewPath;
                        }
                        sprintf(fullPath, "%s/%s", startDir, currentDir->d_name);
                        readDirectory(fullPath, optFicCaches, optExploRecursive);
                    }
                }
            }
            closedir(ptD);
        }
    }
    else{
        printf(RED("Dossier %s inexistant\n"),startDir);
    }

    free(fullPath);
}

int main(int argc, char *argv[]){ // STATUS : RETOURNER 0 LORSQUE PAS DE PROBLEME, RETOURNER 2 SI MOINDRE FICHIER MANQUANT
    char *tabFics[argc];
    int nbFics=0;
    int i, j, lenArg;
    int optFicCaches=0;
    int optExploRecursive=0;

    for(i=1; i<argc; i++){
        // Argument
        if(argv[i][0]=='-'){
            lenArg = (int) strlen(argv[i]);
            if(lenArg==1){
                printf(YELLOW("Erreur : Veuillez spécifier l'option désirée (-a pour lister les fichiers cachés, -R pour explorer récursivement les répertoires).\nUtilisation : myls -a -R\n"));
                return 1;
            }
            for(j=1; j<lenArg; j++){
                if(argv[i][j]=='a'){
                    optFicCaches=1;
                }
                else if(argv[i][j]=='R'){
                    optExploRecursive=1;
                }
                else{
                    printf(YELLOW("Erreur : Seules les options -a (lister les fichiers cachés) et -R (explorer récursivement) sont autorisées.\nUtilisation : myls -a -R\n"));
                    return 1;
                }
            }
        }
        // Fichier
        else{
            tabFics[nbFics++]=argv[i];
        }
    }

    //printf("nbfics: %i, optFicCaches: %i, optRec: %i\n",nbFics,optFicCaches,optExploRecursive);
    if(nbFics==0){
        readDirectory(".", optFicCaches, optExploRecursive);
    }
    else{
        for(i=0; i<nbFics; i++){
            readDirectory(tabFics[i], optFicCaches, optExploRecursive);
        }
    }

    return 0;
}
