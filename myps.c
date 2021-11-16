#include "utils.h"

#include <regex.h>

int readDirectory(){
    DIR *ptD;
    struct dirent *currentDir;

    ptD = opendir("/proc");
    if(ptD){
        regex_t regExpr;
        int compRegExpr;
        char buffer[SIZETAB];
        compRegExpr = regcomp(&regExpr, "^[0-9]*$", 0);
        if(compRegExpr){
            regerror(compRegExpr, &regExpr, buffer, sizeof(buffer));
            fprintf(stderr, YELLOW("Error: %s\n"), buffer);
            exit(EXIT_FAILURE);
        }

        FILE *f = fopen("/proc/uptime", "r");
        if(f==NULL){
            printf("Cannot open file /proc/uptime\n");
            exit(1);
        }
        float systemUptime;
        if(fscanf(f, "%e", &systemUptime)==EOF){
            printf("Can't find systemUptime\n");
            exit(1);
        }
        fclose(f);

        f = fopen("/proc/meminfo", "r");
        if(f==NULL){
            printf("Cannot open file /proc/meminfo\n");
            exit(1);
        }
        int totalMem;
        if(fscanf(f, "MemTotal: %d kB", &totalMem)==EOF){
            printf("Can't find totalMem\n");
            exit(1);
        }
        fclose(f);

        int lenNewPath;
        int lenFullPath = SIZETAB;
        char *fullPath = (char*) malloc(SIZETAB*sizeof(char));
        
        char comm[SIZETAB];
        char state;
        int notUsed, utime, stime, cutime, cstime, starttime;

        struct stat fileStats;
        struct passwd *strtOwner;

        long systemHertz = sysconf(_SC_CLK_TCK);
        int seconds;

        int memProc;

        char line[SIZETAB];
        int vsz;
        int rss;
        
        char linkPath[SIZETAB];
        int linkPathLen;

        char currentTimeString[SIZETAB];
        time_t timestamp = time(NULL);
        struct tm* timeInfos = localtime(&timestamp);
        char timeString[SIZETAB];

        int totalTimeSeconds;

        printf("USER PID %%CPU %%MEM VSZ RSS TTY STAT START TIME COMMAND\n");

        while((currentDir=readdir(ptD)) != NULL){
            compRegExpr = regexec(&regExpr, currentDir->d_name, 0, NULL, 0);
            if(!compRegExpr){
                lenNewPath = strlen(currentDir->d_name)+50;
                if(lenNewPath>lenFullPath){
                    fullPath = (char*) realloc(fullPath, lenNewPath*sizeof(char));
                    lenFullPath = lenNewPath;
                }

                sprintf(fullPath, "/proc/%s/stat", currentDir->d_name);
                f = fopen(fullPath, "r");
                if(f==NULL){
                    printf("Cannot open file %s\n", fullPath);
                    exit(1);
                }
                if(fscanf(f, "%d %s %c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &notUsed, comm, &state, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &notUsed, &utime, &stime, &cutime, &cstime, &notUsed, &notUsed, &notUsed, &notUsed, &starttime)==EOF){
                    printf("Can't find stat informations\n");
                    exit(1);
                }
                fclose(f);

                switch(state){
                    case 'D': printf("\033[1;34m"); break; // D Uninterruptible sleep
                    case 'I': printf("\033[1;36m"); break; // I Idle
                    case 'R': printf("\033[1;35m"); break; // R Running
                    case 'S': printf("\033[1;36m"); break; // S Interruptible sleep
                    case 'T': printf("\033[0;33m"); break; // T Stopped
                    case 'W': printf("\033[0;35m"); break; // W Paging
                    case 'X': printf("\033[1;31m"); break; // X Dead
                    case 'Z': printf("\033[1;32m"); break; // Z Zombie
                    default: printf("\033[0m"); break; // Type inconnu
                }

                // USER
                sprintf(fullPath, "/proc/%s", currentDir->d_name);
                if(stat(fullPath, &fileStats)!=0){
                    printf(YELLOW("Error: %s\n"), strerror(errno));
                    exit(1);
                }
                
                if((strtOwner=getpwuid(fileStats.st_uid))!=NULL){
                    printf("%s", strtOwner->pw_name);
                }
                else{
                    printf("'Unknown user'");
                }

                // PID
                printf(" %s", currentDir->d_name);
                
                // %CPU
                seconds = systemUptime-((float) starttime/systemHertz);
                if(seconds==0) seconds=1;
                printf(" %.1f", 100*(((float) (utime+stime+cutime+cstime)/systemHertz)/seconds));

                // %MEM
                sprintf(fullPath, "/proc/%s/statm", currentDir->d_name);
                f = fopen(fullPath, "r");
                if(f==NULL){
                    printf("Cannot open file %s\n", fullPath);
                    exit(1);
                }
                if(fscanf(f, "%d", &memProc)==EOF){
                    printf("Can't find memProc\n");
                    exit(1);
                }
                fclose(f);
                printf(" %.1f", 100*((float) memProc/totalMem));

                // VSZ
                sprintf(fullPath, "/proc/%s/status", currentDir->d_name);
                f = fopen(fullPath, "r");
                if(f==NULL){
                    printf("Cannot open file %s\n", fullPath);
                    exit(1);
                }
                while(fgets(line, sizeof(line), f)){
                    if(fscanf(f, "VmSize: %d kB", &vsz)==1){
                        printf(" %d", vsz);
                        break;
                    }
                }

                // RSS
                while(fgets(line, sizeof(line), f)){
                    if(fscanf(f, "VmRSS: %d kB", &rss)==1){
                        printf(" %d", rss);
                        break;
                    }
                }
                fclose(f);

                // TTY
                sprintf(fullPath, "/proc/%s/fd/0", currentDir->d_name);
                if((linkPathLen=readlink(fullPath, linkPath, sizeof(linkPath)-1))!=-1){
                    linkPath[linkPathLen]='\0';
                    printf(" %s", linkPath);
                }
                else{
                    printf(" ?");
                }

                // STAT
                printf(" %c", state);

                // START
                if(strftime(currentTimeString, sizeof(currentTimeString), "%b %d %Y", timeInfos)==0){
                    printf(" 'Unknown current date'");
                }
                if(strftime(timeString, 50, "%b %d %Y", localtime(&fileStats.st_ctime))!=0){
                    if(!strcmp(currentTimeString, timeString)){
                        if(strftime(timeString, sizeof(timeString), "%H:%M", localtime(&fileStats.st_ctime))!=0){
                            printf(" %s", timeString);
                        }
                        else{
                            printf(" 'Unknown creation date'");
                        }
                    }
                    else{
                        if(strftime(timeString, sizeof(timeString), "%b%d %Y", localtime(&fileStats.st_ctime))!=0){
                            printf(" %s", timeString);
                        }
                        else{
                            printf(" 'Unknown creation date'");
                        }
                    }
                }
                else{
                    printf(" 'Unknown creation date'");
                }

                // TIME
                totalTimeSeconds = (utime+stime)/systemHertz;
                printf(" %02d:%02d:%02d", totalTimeSeconds/3600, (totalTimeSeconds%3600)/60, (totalTimeSeconds%3600)%60);

                // COMMAND
                sprintf(fullPath, "/proc/%s/status", currentDir->d_name);
                f = fopen(fullPath, "r");
                if(f==NULL){
                    printf("Cannot open file %s\n", fullPath);
                    exit(1);
                }
                if(fscanf(f, "Name: %[^\n]", comm)==EOF){
                    printf("Can't find comm\n");
                    exit(1);
                }
                fclose(f);
                printf(" %s\n", comm);
            }
            else if(compRegExpr == REG_NOMATCH){
                // Nothing to do
            }
            else{
                regerror(compRegExpr, &regExpr, buffer, sizeof(buffer));
                fprintf(stderr, YELLOW("Error: %s\n"), buffer);
                exit(EXIT_FAILURE);
            }
        }
        printf("\033[0m");
        free(fullPath);
        regfree(&regExpr);
        closedir(ptD);
    }
    else{
        printf(RED("Dossier /proc inexistant\n"));
    }

    return 0;
}

int main(void){ // STATUS : RETOURNER 0 LORSQUE PAS DE PROBLEME, RETOURNER 1 SINON
    return readDirectory();
}
