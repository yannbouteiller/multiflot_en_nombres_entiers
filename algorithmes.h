/*

GRAPHE D'ECART :

La construction du graphe d'écart impacte le temps d'execution de l'algorithme de Busacker et Gowen, et son temps d'accès est déterminant pour l'execution de l'algorithme de Bellman-Ford. On veut donc une structure optimisée en temps de construction et optimisée pour Bellman-Ford (liste d'arcs).

---

HEURISTIQUE :

On cherche une heuristique qui trouve une solution réalisable du problème de multiflots en nombres entiers.
L'heuristique proposée est, pour chaque commodité une à une, de déterminer son flot optimal dans le graphe en espérant que cela ne sature pas d'arc mutualisé, rendant ainsi le problème irréalisable.

Afin d'être quasi-certain de trouver une solution réalisable quand il en existe une, une amélioration logique consiste à reprendre la même idée, mais en changeant le coût des arcs : un arc sera gratuit pour l'heuristique s'il n'a pas de capacité mutualisée (c'est à dire une capacité mutualisée infinie), et sera d'autant plus cher que sa capacitée mutualisée est faible. Ainsi, on évite d'emprunter les arcs mutualisés, afin de ne pas les saturer.

*/

#ifndef DEF_ALGO
#define DEF_ALGO

typedef struct ArcEcart ArcEcart;
struct ArcEcart
{
	int i;
	int j;
	double cout;
	int capa;
	Arc* original;
	char inverse; // 1 si sens inverse, 0 si sens direct (par rapport au graphe de départ)
};

typedef struct LiArcs LiArcs; // graphe d'écart codé sous forme de liste d'arcs
struct LiArcs
{
	int taille;
	ArcEcart* liste; // tableau d'ArcEcart
	int tailleChemin;
	ArcEcart* *chemin;
	double coutChemin;
	int capaChemin;
};

void allouer_graphe_ecart(LiArcs* h, Graphe* g);
void detruire_graphe_ecart(LiArcs* h, Graphe* g);
double cout_heuristique(Arc* a, int heuristique, Graphe* g);
void nouvel_arc_ecart(LiArcs* h, int inverse, int i, int j, int capa, int m, Graphe* g, int commodite, int heuristique, double** lagr);
void construire_graphe_ecart(LiArcs* h, Graphe* g, int commodite, int heuristique, double** lagr);
void afficher_graphe_ecart(LiArcs* h);
int coutSupStrict(double a, double b);
int coutInfStrict(double a, double b);
int Bellman_Ford(LiArcs* h, Graphe* g, double* cout, ArcEcart* *pere); // 1 si pas de cycle negatif, 0 sinon.
void afficher_BF(int n, ArcEcart* *pere, double* cout);
int flux(Graphe* g, int commodite);
int chemin_de_cout_min(LiArcs* h, ArcEcart* *pere, double* cout, int n, int commodite); // remplit le chemin dans h
void afficher_chemin(LiArcs* h);
int Busacker_Gowen(Graphe* g, LiArcs* h, double* coutTotalCom, int commodite, int heuristique, double** lagr); // travaille sur h qui doit être préalablement alloué ; si heuristique != 0, le coût des arrêtes est changé par heuristique-mutRestante.
int heuristique_rec(Graphe* g, LiArcs* h, int* tab, double* coutCom, int mutMax);
int heuristique(Graphe* g, LiArcs* h);
double cout_graphe(Graphe* g);
void reinitialiser_flots(Graphe* g);
int solution_realisable(Graphe* g); // teste si g est realisable
int penalites_nulles(Graphe* g, double** lagr); // teste si les penalités sont nulles
double** alloc_lagr(Graphe* g);
void free_lagr(double** lagr, Graphe* g);
int sous_gradients(Graphe* g, LiArcs* h, int IMax, double alpha);


#endif


