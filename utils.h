#pragma once

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define ERR -1

#define AND "&&"
#define OR "||"
#define TUBE "|"
#define REDIRECTSIMPLE ">"
#define REDIRECTFIN ">>"
#define REDIRECTSIMPLEERR "2>"
#define REDIRECTFINERR "2>>"
#define REDIRECTSIMPLEALL ">&"
#define REDIRECTFINALL ">>&"
#define REDIRECTSTDIN "<"

#define OPADD "+"
#define OPMINUS "-"
#define OPDIV "/"
#define OPMUL "*"
#define OPADDEQUAL "+="
#define OPMINUSEQUAL "-="
#define OPDIVEQUAL "/="
#define OPMULEQUAL "*="

#define AJOUTVAR "set"
#define SUPPRVAR "unset"
#define AFFICHEVAR "echo"

#define MAX_CLIENTS 100
#define SIZETAB 2048

#define SSH_MSG_USERAUTH_REQUEST 50
#define SSH_MSG_USERAUTH_FAILURE 51
#define SSH_MSG_USERAUTH_SUCCESS 52

#define SSH_MSG_CHANNEL_REQUEST 98
#define SSH_MSG_CHANNEL_SUCCESS 99
#define SSH_MSG_CHANNEL_FAILURE 100

#define RED(s) "\033[0;31m"s"\033[0m"
#define GREEN(s) "\033[0;32m"s"\033[0m"
#define YELLOW(s) "\033[0;33m"s"\033[0m"
#define BLUE_BOLD(s) "\033[1;34m"s"\033[0m"
#define CYAN_BOLD(s) "\033[1;36m"s"\033[0m"
#define GREEN_BOLD(s) "\033[1;32m"s"\033[0m"
#define MAGENTA_BOLD(s) "\033[1;35m"s"\033[0m"
#define RED_BOLD(s) "\033[1;31m"s"\033[0m"
#define YELLOW_BOLD(s) "\033[1;33m"s"\033[0m"

#define ERRORCHECK(fct,x) if(fct==x) fprintf(stderr, YELLOW("Error: %s\n"), strerror(errno));
#define ERRORCHECKNE(fct,x) if(fct!=x) fprintf(stderr, YELLOW("Error: %s\n"), strerror(errno));
