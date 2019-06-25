#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "graphe.h"
#include "algorithmes.h"

#include "constantes.h"

void allouer_graphe_ecart(LiArcs* h, Graphe* g) // une seule fois pour tout le programme
{
	h->liste = malloc(g->a*2*sizeof(ArcEcart)); // on alloue une taille supérieure à la taille possible des graphes d'écart
	h->chemin = malloc((g->nDepart+1)*sizeof(ArcEcart*)); // le chemin ne doit normalement pas contenir de cycle
}
void detruire_graphe_ecart(LiArcs* h, Graphe* g) // une seule fois pour tout le programme
{
	free(h->liste);
	free(h->chemin);
}

double cout_heuristique(Arc* a, int heuristique, Graphe* g)
{
	if(a->mut == -1) return 0; // capa mut infinie : arc gratuit
	else return heuristique - a->mut + 1;
}

void nouvel_arc_ecart(LiArcs* h, int inverse, int i, int j, int capa, int m, Graphe* g, int commodite, int heuristique, double** lagr)
{
	if(capa < -1)
	{
		printf("ERREUR : arc ecart %d->%d, capa %d (inverse : %d, com : %d)\n", i+1, j+1, capa, inverse, commodite);
		exit(EXIT_FAILURE);
	}

	ArcEcart ae;
	ae.inverse = inverse;
	if(inverse == 0)
	{
		ae.i = i;
		ae.j = j;
		if(heuristique == 0)
		{
			ae.cout = g->inc[i][j]->arcs[m].pcom[commodite-1].cout;
			if(g->inc[i][j]->arcs[m].mut >= 0) ae.cout += lagr[i][j];
		}
		else ae.cout = cout_heuristique(&g->inc[i][j]->arcs[m], heuristique, g);
	}
	else
	{
		ae.i = j;
		ae.j = i;
		if(heuristique == 0)
		{
			ae.cout = -1 * g->inc[i][j]->arcs[m].pcom[commodite-1].cout;
			if(g->inc[i][j]->arcs[m].mut >= 0) ae.cout -= lagr[i][j];
		}
		else ae.cout = -1 * cout_heuristique(&g->inc[i][j]->arcs[m], heuristique, g);
	}
	ae.capa = capa;
	ae.original = &g->inc[i][j]->arcs[m];

	h->liste[h->taille] = ae;
	h->taille++;
}

void construire_graphe_ecart(LiArcs* h, Graphe* g, int commodite, int heuristique, double** lagr) // si heuristique != 0, le cout est calculé en fonction de la capa mut
{
	int supersource = g->nDepart+commodite-1; // indice de la supersource

	h->taille = 0;

	// on commence par s'occuper des voisins sortants de la supersource

	for(int i = 0; i < g->sortants[supersource].nbSort; i++)
	{
		int ii = supersource;
		int jj = g->sortants[supersource].sort[i];
		int capa = g->inc[ii][jj]->arcs[0].pcom[commodite-1].capa;
		int flot = g->inc[ii][jj]->arcs[0].pcom[commodite-1].flot;

		if(capa == -1) // capacité infinie
		{
			nouvel_arc_ecart(h, 0, ii, jj, -1, 0, g, commodite, heuristique, lagr);
		}
		else if(capa - flot >= 1) // sens direct (résiduel)
		{
			nouvel_arc_ecart(h, 0, ii, jj, capa - flot, 0, g, commodite, heuristique, lagr);
		}

		if(flot >= 1) // sens indirect (flot inverse)
		{
			nouvel_arc_ecart(h, 1, ii, jj, flot, 0, g, commodite, heuristique, lagr);
		}
	}

	// ensuite, on s'occupe de tous les autres noeuds (superpuits pointé par ses voisins entrants)

	for(int i = 0; i < g->nDepart; i++) // pour chaque sommet de départ
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj]; // pour chacun de ses voisins sortants

			for(int m = 0; m < g->inc[i][j]->nbArcs; m++) // pour chaque arc multiple i->j
			{
				int capa = g->inc[i][j]->arcs[m].pcom[commodite-1].capa;
				int flot = g->inc[i][j]->arcs[m].pcom[commodite-1].flot;
				
				if(heuristique == 0)
				{
					if(capa == -1) // capacité infinie
					{
						nouvel_arc_ecart(h, 0, i, j, -1, m, g, commodite, heuristique, lagr);
					}
					else if(capa - flot >= 1) // sens direct (résiduel)
					{
						nouvel_arc_ecart(h, 0, i, j, capa - flot, m, g, commodite, heuristique, lagr);
					}
				}
				else // heuristique
				{
					int mut = g->inc[i][j]->arcs[m].mut;
					int flotTotal = g->inc[i][j]->arcs[m].flotTotal;
					int flotMax;
					if(mut == -1) // mut infinie
					{
						if(capa == -1) flotMax = -1;
						else flotMax = capa - flot;
					}
					else // il faut considérer la capacité mutualisée de l'arc
					{
						if(capa == -1) flotMax = mut - flotTotal;
						else // mut et capa ne sont pas infinies
						{
							if(capa - flot <= mut - flotTotal) flotMax = capa - flot;
							else flotMax = mut - flotTotal;
						}
					}
					
					if(flotMax != 0) nouvel_arc_ecart(h, 0, i, j, flotMax, m, g, commodite, heuristique, lagr);
				}

				if(flot >= 1) // sens indirect (flot inverse)
				{
					nouvel_arc_ecart(h, 1, i, j, flot, m, g, commodite, heuristique, lagr);
				}
			}
		}
	}
}

void afficher_graphe_ecart(LiArcs* h)
{
	printf("GRAPHE D'ECART : \n");
	printf("Nombre d'arcs : %d \n", h->taille);
	for(int i = 0; i < h->taille; i++)
	{
		if(h->liste[i].inverse == 1) printf("(arc inverse) ");
		printf("%d -> %d : %lf \n", h->liste[i].i+1, h->liste[i].j+1, h->liste[i].cout);
	}
}

int coutSupStrict(double a, double b)
{
	if(fabs(a-b)<EPSILON_COUT) return 0; // egalité
	else if(a>b) return 1;
	else return 0;
}

int coutInfStrict(double a, double b)
{
	if(fabs(b-a)<EPSILON_COUT) return 0; // egalité
	else if(a<b) return 1;
	else return 0;
}

int Bellman_Ford(LiArcs* h, Graphe* g, double* cout, ArcEcart* *pere) // les peres sont des arcs à cause des arcs multiples
{
	int n = g->nDepart+2; // sommets de départ + supersource + superpuits
	int source = g->nDepart;
	int nouvelleOperation = 1;

	char infini[n]; // 1 lorsque le coût est infini, 0 sinon

	for(int i = 0; i < n; i++)
	{
		cout[i] = -1;
		infini[i] = 1;
		pere[i] = NULL;
	}
	infini[source] = 0;
	cout[source] = 0;
	pere[source] = NULL;

	for(int cmpt = 1; cmpt < n; cmpt++)
	{
		if(nouvelleOperation == 0) return 1; // pas d'optimisation à la boucle précédente : on a trouvé un CPM
		else nouvelleOperation = 0;
		
		for(int ii = 0; ii < h->taille; ii++)
		{
			int i = h->liste[ii].i;
			if(i >= g->nDepart)
			{
				if(i >= g->nDepart + g->com) i = n-1; // superpuits
				else i = n-2; // supersource
			}
			int j = h->liste[ii].j;
			if(j >= g->nDepart)
			{
				if(j >= g->nDepart + g->com) j = n-1; // superpuits
				else j = n-2; // supersource
			}

			if(infini[i] == 0)
			{
				if(infini[j] == 1)
				{
					infini[j] = 0;
					cout[j] = cout[i] + h->liste[ii].cout;
					pere[j] = &h->liste[ii];
					nouvelleOperation = 1;
				}
				else if(coutSupStrict(cout[j], cout[i] + h->liste[ii].cout))
				{
					cout[j] = cout[i] + h->liste[ii].cout;
					pere[j] = &h->liste[ii];
					nouvelleOperation = 1;
				}
			}
		}
	}

	for(int ii = 0; ii < h->taille; ii++) // test cycle négatif
	{
		int i = h->liste[ii].i;
		if(i >= g->nDepart)
		{
			if(i >= g->nDepart + g->com) i = n-1; // superpuits
			else i = n-2; // supersource
		}
		int j = h->liste[ii].j;
		if(j >= g->nDepart)
		{
			if(j >= g->nDepart + g->com) j = n-1; // superpuits
			else j = n-2; // supersource
		}

		if(infini[i] == 0)
		{
			if(infini[j] == 1)
			{
				printf("%d->%d : distance d'arrivee infinie mais pas distance de depart\n", i+1, j+1);
				return 0;
			}
			else if(coutSupStrict(cout[j], cout[i] + h->liste[ii].cout)) return 0;
		}
	}

	return 1;
}

void afficher_BF(int n, ArcEcart* *pere, double* cout)
{
	printf("Resultats de Bellman-Ford :\n");
	for(int i = 0; i < n; i++)
	{
		if(pere[i] != NULL) printf("%d : pere = %d, cout = %lf\n",pere[i]->j+1, pere[i]->i+1, cout[i]);
		else
		{
			if(i > n-3) printf("supersource\n");
			else printf("%d : pas de pere\n",i+1);
		}
	}
}

int flux(Graphe* g, int commodite)
{
	int res = 0;

	int i = g->nDepart + commodite - 1;

	for(int ii = 0; ii < g->sortants[i].nbSort; ii++)
	{
		int j = g->sortants[i].sort[ii];
		res += g->inc[i][j]->arcs[0].pcom[commodite-1].capa;
	}

	return res;
}

int chemin_de_cout_min(LiArcs* h, ArcEcart* *pere, double* cout, int n, int commodite)
{
	int i = n-1;
	h->coutChemin = cout[n-1];
	h->capaChemin = -1;
	h->tailleChemin = 0;

	if(h->coutChemin == -1) return 0; // pas de chemin

	int isav = -2;

	while(pere[i] != NULL)
	{
		int capa = pere[i]->capa;
		
		if(capa != -1)
		{
			if(h->capaChemin == -1) h->capaChemin = capa;
			else if(h->capaChemin > capa) h->capaChemin = capa;
		}

		h->chemin[h->tailleChemin] = pere[i];
		h->tailleChemin++;

		if(pere[i]->i > n-3) return 1; // supersource
		else
		{
			if(isav == pere[i]->i)
			{
				printf("ERREUR : boucle\n");
				exit(EXIT_FAILURE);
			}
			isav = i;
			i = pere[i]->i;
		}
	}

	return 0;
}

void afficher_chemin(LiArcs* h)
{
	printf("Chemin de cout minimum : ");
	for(int i = h->tailleChemin-1; i >= 0; i--)
	{
		printf("%d->%d ", h->chemin[i]->i+1, h->chemin[i]->j+1);
	}
	printf("\ncapa : %d\ncout : %lf\n", h->capaChemin, h->coutChemin);
}

int Busacker_Gowen(Graphe* g, LiArcs* h, double* coutTotalCom, int commodite, int heuristique, double** lagr)
{
	int F = flux(g, commodite);

	int Q = 0;
	*coutTotalCom = 0;

	int n = g->nDepart+2; // taille des tableaux sur lesquels on travaille (avec 1 supersource et 1 superpuits)

	double cout[n];
	ArcEcart* pere[n];
	
	int savQ = 0;
	while(Q != F)
	{
		construire_graphe_ecart(h, g, commodite, heuristique, lagr);
		//afficher_graphe_ecart(h);
		
		if(Bellman_Ford(h, g, cout, pere) == 0)
		{
			printf("Cycle negatif detecte\n");
			return 0;
		}
		//afficher_BF(n, pere, cout);
		if(chemin_de_cout_min(h, pere, cout, n, commodite) == 1)
		{
			//afficher_chemin(h);

			int fluxSup = h->capaChemin;
			
			if(fluxSup < -1) {printf("Erreur : flux sup < -1\n");exit(EXIT_FAILURE);}

			if(F - Q < fluxSup || fluxSup == -1)
			{
				fluxSup = F - Q;
				Q = F;
			}
			else Q += fluxSup;
			
			if(Q == savQ)
			{
				printf("ERREUR : le flot total n'a pas augmente (commodite %d)\n", commodite);
				exit(EXIT_FAILURE);
			}
			else savQ = Q;

			*coutTotalCom += h->coutChemin * fluxSup;

			for(int i = 0; i < h->tailleChemin; i++) // mise à jour des flots
			{
				if(h->chemin[i]->inverse == 0)
				{
					h->chemin[i]->original->pcom[commodite-1].flot += fluxSup;
					h->chemin[i]->original->flotTotal += fluxSup;
				}
				else
				{
					h->chemin[i]->original->pcom[commodite-1].flot -= fluxSup;
					h->chemin[i]->original->flotTotal -= fluxSup;
				}
			}
		}
		else
		{
			printf("plus court chemin non trouve\n");
			return 0; // chemin non trouvé
		}
	}

	return 1;
}

int heuristique_rec(Graphe* g, LiArcs* h, int* tab, double* coutCom, int mutMax)
{
	int nouveauTab[g->com];
	for(int i = 0; i < g->com; i++) nouveauTab[i] = tab[i];
	
	for(int k = 1; k <= g->com; k++)
	{
		for(int i = 0; i < g->com; i++) // index sur le tableau
		{
			if(tab[i] == k) break; // on vient de tomber sur la commodité cherchée : elle a déjà été visitée
			else if(tab[i] == 0) // on vient de tomber sur la première case vide du tableau
			{
				nouveauTab[i] = k; // on renseigne cette commodité dans le nouveau tableau
				if(heuristique_rec(g, h, nouveauTab, coutCom, mutMax) == 0) break; // et on appelle récursivement l'algo pour la commodité suivante
				else return 1; // si on a trouvé une solution, il est inutile de continuer l'algo
			}
		}
	}
	
	if(tab[g->com - 1] == 0) return 0;
	
	// à ce niveau de l'algorithme, tab contient une combinaison de toutes les commodités
	
	printf("Combinaison des commodites testee : ");
	for(int i = 0; i < g->com; i++) printf("%d ", tab[i]);
	printf("\n");

	for(int i = 0; i < g->com; i++)
	{
		if(Busacker_Gowen(g, h, coutCom, tab[i], mutMax, NULL) == 0)
		{
			printf("L'heuristique n'est pas parvenue a determiner une solution realisable.\n");
			reinitialiser_flots(g);
			return 0;
		}
		else printf("Commodite %d ok.\n", tab[i]);
	}
	
	return 1;
}

int heuristique(Graphe* g, LiArcs* h)
{
	// on commence par parcourir le graphe afin d'en déterminer l'arc de capacité mutualisé max
	// la formule du coût d'un arc est mutMaxGraphe - mutArc + 1, et 0 pour les arcs non-mutualisés
	
	int mutMax = -1;
	double coutCom = 0; // n'a pas vraiment de sens ici, mais on en a besoin en paramètre de Busacker et Gowen
	
	for(int i = 0; i < g->nDepart; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			
			for(int k = 0; k < g->inc[i][j]->nbArcs; k++)
			{
				if(g->inc[i][j]->arcs[k].mut > mutMax) mutMax = g->inc[i][j]->arcs[k].mut;
			}
		}
	}
	
	printf("Capa mut max : %d\n", mutMax);
	
	int tab[g->com];
	for(int i = 0; i < g->com; i++) tab[i] = 0;
	
	return heuristique_rec(g, h, tab, &coutCom, mutMax);
}

double cout_graphe(Graphe* g)
{
	double resultat = 0;
	
	for(int i = 0; i < g->nDepart; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
			{
				for(int c = 0; c < g->com; c++)
				{
					resultat += g->inc[i][j]->arcs[m].pcom[c].cout * g->inc[i][j]->arcs[m].pcom[c].flot;
				}
			}
		}
	}
	
	return resultat;
}

void reinitialiser_flots(Graphe* g)
{
	for(int i = 0; i < g->n; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
			{
				g->inc[i][j]->arcs[m].flotTotal = 0;
				for(int c = 0; c < g->com; c++)
				{
					g->inc[i][j]->arcs[m].pcom[c].flot = 0;
				}
			}
		}
	}
}

int solution_realisable(Graphe* g)
{
	for(int i = 0; i < g->nDepart; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
			{
				if(g->inc[i][j]->arcs[m].mut >= 0)
				{
					if(g->inc[i][j]->arcs[m].flotTotal > g->inc[i][j]->arcs[m].mut) return 0;
				}
			}
		}
	}
	
	return 1;
}

int penalites_nulles(Graphe* g, double** lagr)
{
	for(int i = 0; i < g->nDepart; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
			{
				if(g->inc[i][j]->arcs[m].mut >= 0)
				{
					double p = lagr[i][j] * (g->inc[i][j]->arcs[m].flotTotal - g->inc[i][j]->arcs[m].mut);
					if(coutInfStrict(p, 0) == 1 || coutSupStrict(p, 0) == 1) return 0;
				}
			}
		}
	}
	
	return 1;
}

double** alloc_lagr(Graphe* g)
{
	double** lagr = malloc(g->nDepart * sizeof(double *));
	if(lagr == NULL)
	{
		printf("ERREUR de malloc pour lagr\n");
		exit(EXIT_FAILURE);
	}
	lagr[0] = malloc(g->nDepart * g->nDepart * sizeof(double));
	if(lagr[0] == NULL)
	{
		printf("ERREUR de malloc pour lagr\n");
		exit(EXIT_FAILURE);
	}
	for(int i = 1; i < g->nDepart; i++) lagr[i] = lagr[0] + i * g->nDepart;
	
	return lagr;
}

void free_lagr(double** lagr, Graphe* g)
{
	free(lagr[0]);
	free(lagr);
}

int sous_gradients(Graphe* g, LiArcs* h, int IMax, double alpha)
{
	FILE* fic = NULL;
	fic = fopen(NOM_FICHIER_LOG, "w+");
	if(fic == NULL)
	{
		printf("ERREUR : echec lors de la creation/ouverture du fichier logs\n");
		exit(EXIT_FAILURE);
	}
	
	double** lagr = alloc_lagr(g); // allocation de la matrice des multiplicateurs lagrangiens
	
	printf("HEURISTIQUE :\n");
	if(heuristique(g,h) == 0)
	{
		printf("L'heurstique a echoue a trouver une solution realisable.\n");
		return 0;
	}

	double Zub = cout_graphe(g);
	
	printf("Cout de la solution heuristique (Zub) : %lf\n", Zub);
	
	// initialisation des multiplicateurs lagrangiens :

	for(int i = 0; i < g->nDepart; i++)
	{
		for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
		{
			int j = g->sortants[i].sort[jj];
			for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
			{
				if(g->inc[i][j]->arcs[m].mut >= 0) lagr[i][j] = 0; // on initialise seulement les arcs mutualisés, car on n'utilisera pas les autres (pour les systèmes d'exploitation optimisés en gestion de mémoire)
			}
		}
	}
	
	// résolution du problème dual :
	printf("PROBLEME DUAL :\n");
	
	double Zlb;
	double ZlbMax = -1;

	int I = 0;
	while(I < IMax)
	{
		reinitialiser_flots(g);
		
		Zlb = 0;

		for(int commodite = 1; commodite <= g->com; commodite++) // pour chaque commodite
		{
			double coutTotalCom = 0;
	
			if(Busacker_Gowen(g, h, &coutTotalCom, commodite, 0, lagr) == 0)
			{
				printf("ERREUR : L'algorithme de Busacker et Gowen a echoue pour la commodite %d.\n", commodite);
				return 0; 
			}
	
			Zlb += coutTotalCom;
		}
		
		if(coutInfStrict(Zlb,ZlbMax) == 1) I++;
		else if(coutSupStrict(Zlb,ZlbMax) == 1)
		{
			ZlbMax = Zlb;
			I = 0;
		}
		else I++;
		
		fprintf(fic, "%lf\t%lf\n", Zlb, ZlbMax);
		
		if(solution_realisable(g) == 1) // si on a trouvé une solution réalisable du problème initial
		{
			printf("On a trouve une solution realisable.\n");
			if(penalites_nulles(g, lagr) == 1)
			{
				printf("On a trouve une solution optimale. Cout total : %lf\n", Zlb);
				free_lagr(lagr, g);
				return 1;
			}
			else printf("(non opti)");
		}
		else printf("(non realisable)");
		
		printf(" Zlb:%lf (ZlbMax:%lf)\n", Zlb, ZlbMax);
	
		// mise à jour des multiplicateurs lagrangiens :
	
		double sommeCarresG = 0;
		for(int i = 0; i < g->nDepart; i++)
		{
			for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
			{
				int j = g->sortants[i].sort[jj];
				for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
				{
					if(g->inc[i][j]->arcs[m].mut >= 0)
					{
						double G = g->inc[i][j]->arcs[m].flotTotal - g->inc[i][j]->arcs[m].mut;
						sommeCarresG += (G * G);
					}
				}
			}
		}
	
		double pas = alpha * (Zub - Zlb) / sommeCarresG;
		if(coutInfStrict(pas, 0) == 0 && coutSupStrict(pas, 0) == 0)
		{
			printf("ERREUR : le pas est descendu en dessous de la limite negligeable.\n");
			free_lagr(lagr, g);
			return 0;
		}
	
		for(int i = 0; i < g->nDepart; i++)
		{
			for(int jj = 0; jj < g->sortants[i].nbSort; jj++)
			{
				int j = g->sortants[i].sort[jj];
				for(int m = 0; m < g->inc[i][j]->nbArcs; m++)
				{
					if(g->inc[i][j]->arcs[m].mut >= 0)
					{
						double G = g->inc[i][j]->arcs[m].flotTotal - g->inc[i][j]->arcs[m].mut;
						double val = lagr[i][j] + pas * G;
						if(val < 0) lagr[i][j] = 0;
						else lagr[i][j] = val;
					}
				}
			}
		}
	}

	// on n'est pas parvenu à optimiser : l'algo se relance avec alpha = alpha/2

	printf("La solution ne s'est pas amelioree pendant %d iterations avec alpha = %lf.\n", IMax, alpha);
	
	free_lagr(lagr, g);
	fclose(fic);
	return 0;
}


