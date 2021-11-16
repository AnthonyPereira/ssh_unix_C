#include "utils.h"
#include "decoupeCommande.h"

enum Type {INT, FLOAT, STRING};

typedef struct Variable Variable;
struct Variable{
    struct Variable* precedent;
    struct Variable* suivant;
    enum Type type;
    int tailleVar;
    char *name;
    int valInt;
    float valFloat;
    char *valString;
};

void afficheVar(Variable *var);
/* affiche la variable*/
void afficheAllVar(Variable *var,int level);
/*affiche toute les variables*/
int haveEqualSign(char *cmd);
/*dit si il y a le signe = dans le chaine*/
int isNum(char *cmd);
/*dit si c'est un int / float / string*/
char* whatOp(char *op);
/* donne l'operation si il y en a une*/
char* whatOpEqual(char *opEqual);
/*donne operation coller au =*/
int nbCharVar(char* str);
/*calcul le nombre de caractere du nom de la variable*/
Variable *takeVar(Variable *chaineVar, char* nomVar, int level);
/*retourne la variable qui a le nomVar*/
char* enleveCarac(char* str, char carac);/* enleve tout les carac dans str*/
char *enleveDoubleCarac(char *str, char carac);
/*enlever les caractere quand ils sont plus de 2*/
char* transform$(Variable* chaineVar, char* strWithDollar);
/* transforme les $nomVar avec ce que celle ci veut dire*/
char* transformEchoToStr(Variable *chaineVar, char* str);/* transforme le str en quelque chose qui donne le echo*/
Variable* supprVar(Variable *chaineVar, char* nomVar);/*suppr une variable*/
float* doOp(char* declarationOp);/* fait une operation demander dans la chaine de caractere ex : "4 + 4" */
Variable *modifVar(Variable *var, char* valeur);/*modifie la variable avec la nouvelle valeur*/
Variable* newVariable(Variable *chaineVar, char *declarationVar);/*creer une nouvelle variable*/
Variable* siEqual(Variable *chaineVar, char* chaineWithEqual);/* verifie se qu'il faut faire si un set avec un = dedans est donner*/
Variable *ajoutVarSansInit(Variable *chaineVar,char *nomVar);/*creer une nouvelle variable sans l'initialiser*/
void supprAllVar(Variable *var,int level);/*supprimer toute les variables*/
