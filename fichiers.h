#ifndef DEF_FICH
#define DEF_FICH

#include "graphe.h"

int charger_probleme(Probleme *p, int instance); // charge les paramètres du problème - renvoie 1 si ok, 0 si echec
int compter_lignes(FILE* fic); // renvoie le nombre de lignes du fichier

#endif
