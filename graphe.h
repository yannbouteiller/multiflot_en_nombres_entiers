#ifndef DEF_GRAPHE
#define DEF_GRAPHE

typedef struct SMut SMut;
struct SMut
{
	int pm; // nom du pointeur
	int cm; // capacité mutualisée
};

typedef struct SSup SSup;
struct SSup
{
	int n; // noeud
	int com; // commodité
	int sup; // supply
};

typedef struct SArc SArc;
struct SArc
{
	int arc; // nom arc
	int i;
	int j;
	int com; // commodité
	double cout; // coût
	int capa; // capacité
	int pm; // pointeur mut
};

typedef struct Probleme Probleme;
struct Probleme
{
    int n; // nombre de noeuds
    int a; // nombre d'arcs
    int ac; // nombre d'arcs avec capa mut
    int com; // nombre de commodités

    int tailleMut;
    int tailleSup;
    int tailleArc;

    SMut* mut; // mut[ac] /!\ : la case 0 correspond au pointeur mut 1, la case 1 au pointeur 2 etc
    SSup* sup; // node supply
    SArc* arc; // arcs
};

// (/!\ nommage : [i] dans le graphe représente [i+i] dans le fichier d'origine)

typedef struct ArcCommod ArcCommod; // décrit les propritétés d'une commodité sur un arc
struct ArcCommod
{
	int capa; // -1 pour infini
	double cout;
	int flot; // flot passant par cet arc
};

typedef struct Arc Arc; // arcs de la matrice d'adjascence
struct Arc
{
	int nomArc; // nom de l'arc
	int mut; // capacité mutualisée de l'arc : -1 si infinie
	int flotTotal; // flot total passant par l'arc
	ArcCommod* pcom; // propriétés commodités (autant de cases que de commodités dans le problème pour ne pas avoir à chercher)
};

typedef struct TArc TArc; // tableau d'arcs (pour gérer les arcs multiples et les listes de voisins)
struct TArc
{
	int nbArcs; // nombre d'arcs
	Arc* arcs; // propriétés commodités
};

typedef struct TVois TVois; // tableau de voisins sortants
struct TVois
{
	int nbSort; // nombre de voisins sortants
	int* sort; // voisins sortants
};

typedef struct Graphe Graphe;
struct Graphe
{
	int n; // nombre de noeuds
	int nDepart; // nombre de noeuds du graphe sans supersources/superpuits
    	int a; // nombre d'arcs
    	int ac; // nombre d'arcs avec capa mut
    	int com; // nombre de commodités

    	TArc** *inc; // matrice d'incidence (tableaux d'Arcs car arcs multiples autorisés, NULL si pas d'arc (économie mémoire))
    	TVois* sortants; // listes des voisins sortants de chaque noeud (utile pour optimiser le temps de recherche dans inc)
};

int p2mut(int pointeurMut, Probleme* p); // renvoie la valeur mut du pointeur mut
void exit_erreur(int err);
void ajouter_arc_ou_propriete(int i, int j, int nomArc, int mut, int com, int capa, double cout, Graphe* g);
void generer_graphe(Probleme *p, Graphe *g); // génère un Graphe à partir du Probleme (pointeur alloué dynamiquement)
void afficher_graphe(Graphe *g);
void afficher_mut(Graphe *g);
void afficher_probleme(Probleme* p);


#endif



