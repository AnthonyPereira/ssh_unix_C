#include "variableLocal.h"

// Méthodes affichage
void afficheVar(Variable *var){
    if(var->type == STRING){
        printf("%s = %s\n", var->name, var->valString);
    }
    else if(var->type == FLOAT){
        printf("%s = %f\n", var->name, var->valFloat);
    }
    else if(var->type == INT){
        printf("%s = %d\n", var->name, var->valInt);
    }
}

void afficheAllVar(Variable *var,int level){
    if(var != NULL){
        if(level==-1 || level == 0){
            afficheAllVar(var->precedent, -1);
        }
        afficheVar(var);
        if(level == 1 || level == 0){
            afficheAllVar(var->suivant, 1);
        }
    }
}
// ------- Fin affichage -------


// Méthodes de comparaison
int haveEqualSign(char *cmd){
    while(*cmd != '\0'){
        if(*cmd == '='){
            return 1;
        }
        cmd++;
    }
    return 0;
}

int isNum(char *cmd){
    /*
        retourne 0 si pas numerique
        retourne 1 si float
        retourne 2 si int
    */
    int hasComa = 0;
    while(*cmd != '\0'){
        if((*cmd<'0' || *cmd>'9') && *cmd != '.'){
            return 0;
        }
        if(*cmd == '.'){
            hasComa = 1;
        }
        cmd++;
    }
    if(hasComa){
        return 1;
    }
    return 2;
}

char* whatOp(char *op){
    while(*op != '\0'){
        if(verifEqual(op,OPADD)){
            return OPADD;
        }
        else if(verifEqual(op,OPDIV)){
            return OPDIV;
        }
        else if(verifEqual(op,OPMINUS)){
            return OPMINUS;
        }
        else if(verifEqual(op,OPMUL)){
            return OPMUL;
        }
        op++;
    }
    return NULL;
}

char* whatOpEqual(char *opEqual){
    while(*opEqual != '\0'){
        if(verifEqual(opEqual,OPADDEQUAL)){
            return OPADDEQUAL;
        }
        else if(verifEqual(opEqual,OPDIVEQUAL)){
            return OPDIVEQUAL;
        }
        else if(verifEqual(opEqual,OPMINUSEQUAL)){
            return OPMINUSEQUAL;
        }
        else if(verifEqual(opEqual,OPMULEQUAL)){
            return OPMULEQUAL;
        }
        opEqual++;
    }
    return NULL;
}


int nbCharVar(char* str){
    int stop=0, dollarAppear=0, cpt=0;
    while(!stop && *str != '\0'){
        if(dollarAppear && *str !=' '){
            cpt++;
        }
        if(dollarAppear && (*str ==' ' || *str == '\0')){
            stop = 1;
        }
        
        if(*str == '$'){
            
            dollarAppear = 1;
        }
        str++;
    }
    return cpt;
}

Variable *takeVar(Variable *chaineVar, char* nomVar, int level){
    Variable *ret=NULL;
    if(chaineVar == NULL){
        return ret;
    }
    if(strcmp(chaineVar->name,nomVar)==0){
        return chaineVar;
    }
    if(level == -1 || level == 0){
        ret = takeVar(chaineVar->precedent,nomVar,-1);
    }
    if(ret == NULL && (level == 1 || level == 0)){
        ret = takeVar(chaineVar->suivant,nomVar,1);
    }
    return ret;
}
// ------- Fin comparaison -------


// Méthodes de formatage
char *enleveCarac(char *str, char carac){
    char* s=str;
    char* ret=malloc(sizeof(char)*(strlen(str)+1));
    int cpt=0;
    while(*s != '\0'){
        if(*s != carac){
            *(ret+cpt) = *s;
            cpt++;
        }
        s++;
    }
    *(ret+cpt) = '\0';
    return ret;
}

char *enleveDoubleCarac(char *str, char carac){
    char* s=str;
    char* ret=malloc(sizeof(char)*strlen(str));
    int cpt=0;
    while(*s != '\0'){
        if(*s != carac && *(s+1)!= carac){
            *(ret+cpt) = *s;
            cpt++;
        }
        s++;
    }
    *(ret+cpt) = '\0';
    return ret;
}

char* transform$(Variable* chaineVar, char* strWithDollar){
    char *s, *rect;
    Variable *var;
    int nbCarVar, nbCar, cptStr=0;
    char* ret=malloc(sizeof(char)*nbCarac(strWithDollar)+1);
    char* newStr=malloc(sizeof(char)*nbCarac(strWithDollar)+1);
    ret = strcpy(ret, strWithDollar);
    newStr = strcpy(newStr, strWithDollar);
    while(*(newStr+cptStr) != '\0'){
        if(*(newStr+cptStr) == '$'){
            rect = newStr+cptStr;
            nbCar = nbCarac(ret);
            nbCarVar= nbCharVar(rect);
            char nomVar[nbCarVar];
            s = rect+1;
            int i;
            for(i=0; i<nbCarVar; i++){
                nomVar[i] = *(s+i);
            }
            nomVar[i] = '\0';
            var = takeVar(chaineVar, nomVar,0);
            if(var == NULL){ 
                return NULL;
            }
            char* replace = malloc(sizeof(char)*var->tailleVar+1);
            if(var->type == INT){
                sprintf(replace,"%d",var->valInt);
            }
            else if(var->type == FLOAT){
                sprintf(replace,"%f", var->valFloat);
                *(replace+var->tailleVar) = '\0';
            }
            else if(var->type == STRING){
                free(replace);
                replace = var->valString;
            }
            int cpt = 0, dejaFait = 0, cptSave = 0;
            cptStr = cptSave;
            ret = realloc(ret, sizeof(char)*((nbCar-(nbCarVar+1))+strlen(replace)+2));
            while(*(newStr+cptStr) != '\0'){
                if(*(newStr+cptStr) == '$' && !dejaFait){
                    int cptRepl = 0;
                    while(*(replace+cptRepl) != '\0'){
                        *(ret+cpt) = *(replace+cptRepl);
                        cpt++;
                        cptRepl++;
                    }
                    cptStr+=nbCarVar+1;
                    if(*(newStr+cptStr) == '\0'){
                        *(ret+cpt) = '\0';
                    }
                    while(cpt<cptStr){
                        *(ret+cpt) = ' ';
                        cpt++;
                    }
                    dejaFait = 1;
                    cptSave = cptStr;
                }
                else{
                    *(ret+cpt) = *(newStr+cptStr);
                    cpt++;
                    cptStr++;
                }
            }
            cptStr = cptSave;
            if(var->type != STRING){
                free(replace);
            }
            newStr = realloc(newStr, sizeof(char)*(strlen(ret)+1));
            newStr = strcpy(newStr, ret);
        }
        else {
            cptStr++;
        }
    }
    return newStr;

}

char* transformEchoToStr(Variable *chaineVar, char* str){
    str = transform$(chaineVar, str);
    if(str == NULL){
        return "Une variable n'est pas reconnue";
    }
    return str;
}
// ------- Fin formatage -------


// Méthodes usuelles
Variable* supprVar(Variable *chaineVar, char* nomVar){
    Variable *var = takeVar(chaineVar, nomVar,0);
    if (var == NULL){
        return NULL;
    }
    if(var->precedent != NULL){
        var->precedent->suivant = var->suivant;
    }
    if(var->suivant != NULL){
        var->suivant->precedent = var->precedent;
    }
    if(var->suivant == NULL && var->precedent == NULL){
        chaineVar = NULL;
    }
    printf("La variable a été supprimée\n");
    return chaineVar;
}


float* doOp(char* declarationOp){
    char *op, *bfop, *afop;
    int whatIsBfop, whatIsAfop;
    float bf, af, *ret = malloc(sizeof(float));
    op=whatOp(declarationOp);
    if(op == NULL){
        return ret;
    }
    bfop = takeBeforeSign(declarationOp, op);
    afop = takeAfterSign(declarationOp,op);
    bfop = enleveCarac(bfop, ' ');
    afop = enleveCarac(afop, ' ');
    whatIsAfop = isNum(afop);
    whatIsBfop = isNum(bfop);
    if(!whatIsBfop || !whatIsAfop){
        return ret;
    }
    bf = atof(bfop);
    af = atof(afop);
    if(verifEqual(op, OPADD)){
        *ret = bf + af;
    }
    else if(verifEqual(op, OPMINUS)){
        *ret = bf - af;
    }
    else if(verifEqual(op, OPMUL)){
        *ret = bf * af;
    }
    else if(verifEqual(op, OPDIV)){
        *ret = bf / af;
    }
    return ret;
}

Variable *modifVar(Variable *var, char* valeur){
    int typ = isNum(valeur);
    int taille = strlen(valeur);
    var->tailleVar = taille;
    if(typ==0){
        var->type = STRING;
        var->valString = valeur;
    }
    else if(typ == 1){
        var->type = FLOAT;
        var->valFloat = atof(valeur);
    }
    else{
        var->type = INT;
        var->valInt = atoi(valeur);
    }
    printf("La variable a été modifiée\n");
    return var;
}


Variable* newVariable(Variable *chaineVar, char *declarationVar){
    char *valeur;
    int typ, taille;
    Variable *ret = malloc(sizeof(Variable));
    ret->name = takeBeforeSign(declarationVar,"=");
    valeur = takeAfterSign(declarationVar,"=");
    typ = isNum(valeur);
    taille = strlen(valeur);
    ret->tailleVar = taille;
    if(typ==0){
        ret->type = STRING;
        ret->valString = valeur;
    }
    else if(typ == 1){
        ret->type = FLOAT;
        ret->valFloat = atof(valeur);
    }
    else{
        ret->type = INT;
        ret->valInt = atoi(valeur);
    }
    if(chaineVar != NULL){
        ret->precedent = chaineVar->precedent;
        chaineVar->precedent = ret;
        ret->suivant = chaineVar;
    }
    else{
        ret->precedent = NULL;
        ret->suivant = NULL;
    }
    printf("La variable a été créée\n");
    return ret;
}

Variable* siEqual(Variable *chaineVar, char* chaineWithEqual){
    Variable *ret;
    char *isOp;
    float *result;
    char *replace = malloc(sizeof(char)*strlen(chaineWithEqual)+1);
    char *resultat;
    *replace = '\0';
    char *bfEqual=takeBeforeSign(chaineWithEqual, "=");
    char *afEqual=takeAfterSign(chaineWithEqual, "=");
    afEqual = transform$(chaineVar, afEqual);
    bfEqual = enleveCarac(bfEqual, ' ');
    afEqual = enleveCarac(afEqual, ' ');
    isOp = whatOp(afEqual);
    ret = takeVar(chaineVar, bfEqual,0);
    resultat = malloc(sizeof(char)*strlen(afEqual));
    if(isOp != NULL){
        result = doOp(afEqual);
        if(result == NULL){
            if(strcmp(isOp, OPADD)){
                char *bfOp = takeBeforeSign(afEqual, OPADD);
                char *afOp = takeAfterSign(afEqual, OPADD);
                resultat = strcat(resultat, bfOp);
                resultat = strcat(resultat, afOp);
            }
            else{
                resultat = afEqual;
            }
        }
        else{
            resultat = malloc(sizeof(char)*50);        
            sprintf(resultat,"%f", *result);
            free(result);
        }
    }
    else{
        if(ret != NULL){
            replace = afEqual;
        }
    }
    if(ret != NULL){
        if(isOp != NULL){
            free(replace);
            replace = resultat;
        }
        return modifVar(ret, replace);
    }
    else{
        strcat(replace, bfEqual);
        strcat(replace,"=");
        if(isOp != NULL){
            strcat(replace, resultat);
        }
        else{
            strcat(replace, afEqual);
        }
        return newVariable(chaineVar,replace);
    }
}

Variable *ajoutVarSansInit(Variable *chaineVar,char *nomVar){
    Variable *ret = malloc(sizeof(Variable));
    ret->name = nomVar;
    ret->precedent = chaineVar->precedent;
    chaineVar->precedent = ret;
    ret->suivant = chaineVar;
    printf("La variable a été créée\n");
    return ret;
}

void supprAllVar(Variable *var,int level){
    if(var != NULL){
        if(level==-1 || level == 0){
            supprAllVar(var->precedent, -1);
        }
        if(level == 1 || level == 0){
            supprAllVar(var->suivant, 1);
        }
        supprVar(var, var->name);
    }
}
// ------- Fin usuelles -------


/*
int main(){
    Variable *var = malloc(sizeof(Variable));
    var->name = "test";
    var->type = STRING;
    var->valString = "youpi";
    var->tailleVar = 5;
    var->suivant = malloc(sizeof(Variable));
    var->suivant->precedent = var;
    var->suivant->name = "test2";
    var->suivant->type = FLOAT;
    var->suivant->valFloat = 4.5;
    var->suivant->tailleVar = 3;
    var = siEqual(var, "a= 4 + 3");
    afficheAllVar(var,0);

}
*/
