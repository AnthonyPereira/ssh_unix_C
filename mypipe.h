#include "utils.h"
#include "decoupeCommande.h"

void createTabArgs(char** tab[], int taille);/* cree le tableau avec les differentes commandes ainsi que les tubes*/
void afficheTab(char** tab[], int taille);/*affiche le tableau*/
void resetTabArg(char** tab[], int taille);/*reinitialise le tableau*/
int pipeSuivant(char **cmdargs[], int i, int max);/* fait le tube suivant*/
void closePipeNotUsed(int (*tabPipe)[2], int actuel);/*ferme les tubes qui ne sont plus utilises*/
void closeAllPipe(int (*tabPipe)[2], int maxPipe);/*ferme tout les tubes*/
char* getCmd(char** cmdargs);/*prend la commande dans le tableau*/
int doPipe(char *inputCmd);/*fait les differents pipes de la commande*/
