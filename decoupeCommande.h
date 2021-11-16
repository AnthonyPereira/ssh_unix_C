#pragma once

#include "utils.h"

int nbCarac(char* cmd);/*compte le nombre de caractere du char**/
int verifEqual(char* str, char* verif);/*verifie si le debut de str est egal a verif*/
int nbSeparator(char *cmd, char* separator);/*compte le nombre de separator dans cmd*/
int hasAnd(char* cmd);/*regarde si il y a des && dans la commande*/
int hasOr(char* cmd);/*regarde si il y a des || dans la commande*/
char* firstSeparator(char* cmd);/* retourne le 1er separateur de cmd*/
char* takeBeforeSign(char* cmd, char* sign);/*dans cmd prend avant la premier chaine sign*/
char* takeAfterSign(char* cmd, char* sign);/*dans cmd prend apres la premier chaine sign*/
char** listCommand(char* cmd, char* cutting, int nbCutting, int nbChar);/*retourne un tableau de toute les commande demandé, separé par le signe cutting*/
int hasRedirection(char * cmd);/*retourne le nombre de direction de cmd*/
char* firstRedirection(char *cmd);/*regarde la 1ere redirection de cmd*/
