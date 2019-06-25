/*
Afin de résoudre le problème à l'aide de Bellman-Ford, on rajoute au graphe une supersource et un superpuits par commodité. La capacité sur ses arcs représente alors les propriétés de supply des sources et des puits.
*/

#include <stdlib.h>
#include <stdio.h>

#include "graphe.h"

int p2mut(int pointeurMut, Probleme* p)
{
	if(pointeurMut <= 0) return -1; // pas de capa mut
	else
	{
		for(int i = 0; i < p->tailleMut; i++) // a priori on n'est pas sûr que mut[i] soit le pointeur i;
		{
			if(p->mut[i].pm == pointeurMut) return p->mut[i].cm;
		}

		return -1;
	}
}

void exit_erreur(int err)
{
	switch(err)
	{
		case 1:
		printf("ERREUR : echec de malloc()\n");
		break;

		case 2:
		printf("ERREUR : echec de realloc()\n");
		break;

		default:
		printf("ERREUR\n");
		break;
	}
	exit(EXIT_FAILURE);
}

void ajouter_arc_ou_propriete(int i, int j, int nomArc, int mut, int com, int capa, double cout, Graphe* g)
{
	if(g->inc[i][j] == NULL) // s'il n'existe pas encore, il faut créer un tableau d'arcs
	{
		TArc* ptemparc = malloc(sizeof(TArc)); // on alloue un nouveau tableau d'arcs
		if(ptemparc != NULL)
		{
			g->inc[i][j] = ptemparc;
			g->inc[i][j]->nbArcs = 1;

			Arc* ptemparc2 = malloc(sizeof(Arc)); // on alloue un premier arc dans le tableau
			if(ptemparc2 != NULL)
			{
				g->inc[i][j]->arcs = ptemparc2;

				g->inc[i][j]->arcs[0].mut = -1; // "pas de capa mut"
				g->inc[i][j]->arcs[0].flotTotal = 0;
				g->inc[i][j]->arcs[0].nomArc = nomArc;

				ArcCommod* ptmp3 = malloc(sizeof(ArcCommod) * g->com);
				if(ptmp3 != NULL)
				{
					g->inc[i][j]->arcs[0].pcom = ptmp3;
					for(int a = 0; a < g->com; a++)
					{
						g->inc[i][j]->arcs[0].pcom[a].capa = 0;
						g->inc[i][j]->arcs[0].pcom[a].cout = 0;
						g->inc[i][j]->arcs[0].pcom[a].flot = 0;
					}
				}
				else exit_erreur(1);
			}
			else exit_erreur(1);
		}
		else exit_erreur(1);
	}

	int iArc = -1;
	for(int a = 0; a < g->inc[i][j]->nbArcs; a++)
	{
		if(g->inc[i][j]->arcs[a].nomArc == nomArc)
		{
			iArc = a;
			break;
		}
	}
	if(iArc == -1) // il faut allouer un nouvel arc (multiple)
	{
		g->inc[i][j]->nbArcs++;
		Arc* ptemparc = realloc(g->inc[i][j]->arcs, sizeof(Arc) * g->inc[i][j]->nbArcs);
		if(ptemparc != NULL)
		{
			g->inc[i][j]->arcs = ptemparc;
			iArc = g->inc[i][j]->nbArcs-1;
			g->inc[i][j]->arcs[iArc].mut = -1;
			g->inc[i][j]->arcs[iArc].flotTotal = 0;
			g->inc[i][j]->arcs[iArc].nomArc = nomArc;

			ArcCommod *ptmp = malloc(g->com * sizeof(ArcCommod));
			if(ptmp != NULL)
			{
				g->inc[i][j]->arcs[iArc].pcom = ptmp;
				for(int a = 0; a < g->com; a++)
				{
					g->inc[i][j]->arcs[iArc].pcom[a].capa = 0;
					g->inc[i][j]->arcs[iArc].pcom[a].cout = 0;
					g->inc[i][j]->arcs[iArc].pcom[a].flot = 0;
				}
			}
			else exit_erreur(1);
		}
		else exit_erreur(2);
	} // ici iArc est défini

	if(mut != -1) g->inc[i][j]->arcs[iArc].mut = mut; // ainsi on n'écrase pas la valeur par un -1

	g->inc[i][j]->arcs[iArc].pcom[com-1].capa = capa;
	g->inc[i][j]->arcs[iArc].pcom[com-1].cout = cout;
}

void generer_graphe(Probleme *p, Graphe *g)
{
	g->n = p->n + 2 * p->com;
	g->nDepart = p->n;
	g->a = p->a + p->tailleSup;
	g->ac = p->ac;
	g->com = p->com;

	void* ptmp = NULL;
	int compteurArcs = p->a;

	ptmp = malloc(sizeof(TArc**) * g->n); // allocation de la matrice d'incidence
	if(ptmp != NULL) g->inc = ptmp;
	else exit_erreur(1);

	TArc* *tarc = malloc(sizeof(TArc*) * g->n * g->n);
	if(tarc == NULL) exit_erreur(1);

	for(int i = 0 ; i < g->n ; i++)
	{
		g->inc[i] = &tarc[i*g->n];

		for(int j = 0 ; j < g->n ; j++)
		{
			g->inc[i][j] = NULL; // initialisation de la matrice à NULL

			// on parcourt les arcs pour trouver toutes les caractéristiques de i->j
			if(j < p->n && i < p->n) for(int ii = 0; ii < p->tailleArc; ii++)
			{
				if(p->arc[ii].i == i+1 && p->arc[ii].j == j+1) // si on est sur i->j
				{
					ajouter_arc_ou_propriete(i, j, p->arc[ii].arc, p2mut(p->arc[ii].pm, p), p->arc[ii].com, p->arc[ii].capa, p->arc[ii].cout, g);
				}
			}
			else // il y a une supersource ou un superpuits dans i ou j
			{
				if(i < p->n) // noeud i vers une supersource ou un superpuits j
				{
					if(j < p->n + p->com) g->inc[i][j] = NULL; // noeud i vers une supersource j
					else // noeud i vers un superpuits j de commodité j-p->n-p->com dans pcom
					{
						for(int ii = 0; ii < p->tailleSup; ii++)
						{
							if(p->sup[ii].n == i+1) // on cherche i dans sup
							{
								if(p->sup[ii].sup < 0) // si i est un puits
								{
									if(p->sup[ii].com == j-p->n-p->com+1) // (de la commodité de j)
									{
										compteurArcs++;
										ajouter_arc_ou_propriete(i, j, compteurArcs, -1, p->sup[ii].com, p->sup[ii].sup * -1, 0, g);
									}
								}
								else g->inc[i][j] = NULL;
							}
						}
					}
				}
				else if(j < p->n) // supersource ou superpuits i vers un noeud j
				{
					if(i >= p->n + p->com) g->inc[i][j] = NULL; // superpuits i vers noeud j
					else // supersource i de commodité i-p->n dans pcom vers noeud j
					{
						for(int ii = 0; ii < p->tailleSup; ii++)
						{
							if(p->sup[ii].n == j+1) // on cherche j dans sup
							{
								if(p->sup[ii].sup > 0) // si j est une source
								{
									if(p->sup[ii].com == i-p->n+1) // (de la commodité de i)
									{
										compteurArcs++;
										ajouter_arc_ou_propriete(i, j, compteurArcs, -1, p->sup[ii].com, p->sup[ii].sup, 0, g);
									}
								}
								else g->inc[i][j] = NULL;
							}
						}
					}
				}
				else g->inc[i][j] = NULL; // pas de noeud du graphe d'origine
			}
		}
	}

	ptmp = malloc(sizeof(TVois) * g->n); // allocation des listes de voisins
	if(ptmp != NULL) g->sortants = ptmp;
	else exit_erreur(1);

	for(int i = 0; i < g->n; i++) g->sortants[i].nbSort = 0;

	for(int i = 0; i < g->n; i++)
	{
		for(int j = 0; j < g->n; j++)
		{
			if(g->inc[i][j] != NULL)
			{
				g->sortants[i].nbSort++;
				if(g->sortants[i].nbSort == 1)
				{
					ptmp = malloc(sizeof(int));
					if(ptmp != NULL)
					{
						g->sortants[i].sort = ptmp;
						g->sortants[i].sort[0] = j;
					}
					else exit_erreur(1);
				}
				else
				{
					ptmp = realloc(g->sortants[i].sort, sizeof(int) * g->sortants[i].nbSort);
					if(ptmp != NULL)
					{
						g->sortants[i].sort = ptmp;
						g->sortants[i].sort[g->sortants[i].nbSort-1] = j;
					}
					else exit_erreur(2);
				}
			}
		}
	}
}

void afficher_probleme(Probleme* p)
{
	printf("PROBLEME :\n");

	printf("nombre de commodites : %d\n", p->com);
	printf("nombre de noeuds : %d\n", p->n);
	printf("nombre d'arcs : %d\n", p->a);
	printf("nombre d'arcs avec capa : %d\n", p->ac);

	printf("tableau MUT :\n");
	{
		for(int i = 0; i < p->tailleMut; i++)
		{
			printf("[%d][%d]\n", p->mut[i].pm, p->mut[i].cm);
		}
	}

	printf("tableau SUP :\n");
	{
		for(int i = 0; i < p->tailleSup; i++)
		{
			printf("[%d][%d][%d]\n", p->sup[i].n, p->sup[i].com, p->sup[i].sup);
		}
	}

	printf("tableau ARC :\n");
	{
		for(int i = 0; i < p->tailleArc; i++)
		{
			printf("[%d][%d][%d][%d][%lf][%d][%d]\n", p->arc[i].arc, p->arc[i].i, p->arc[i].j, p->arc[i].com, p->arc[i].cout, p->arc[i].capa, p->arc[i].pm);
		}
	}
}

void afficher_graphe(Graphe *g)
{
	printf("----- GRAPHE : -----\n\n");

	printf("nombre de commodites : %d\n", g->com);
	printf("nombre de noeuds avec supersources et superpuits : %d\n", g->n);
	printf("nombre de noeuds de départ : %d\n", g->nDepart);
	printf("nombre d'arcs : %d\n", g->a);
	printf("nombre d'arcs avec capa : %d\n", g->ac);

	printf("--- Arcs : ---\n");
	{
		for(int i = 0; i < g->n; i++)
		{
			for(int j = 0; j < g->n; j++)
			{
				if(g->inc[i][j] != NULL)
				{
					printf("\n-- Arcs %d -> %d --\n", i+1, j+1);
					printf("Nombre d'arcs : %d\n", g->inc[i][j]->nbArcs);
					for(int a = 0; a < g->inc[i][j]->nbArcs; a++)
					{
						printf("arc %d:\n", g->inc[i][j]->arcs[a].nomArc);

						printf("Capacite mut :%d\n", g->inc[i][j]->arcs[a].mut);
						printf("Flot total :%d\n", g->inc[i][j]->arcs[a].flotTotal);
						for(int ii = 0; ii < g->com; ii++)
						if(g->inc[i][j]->arcs[a].pcom[ii].capa != 0)
						{
							printf("com %d : cout %lf, capa %d, flot %d\n", ii+1, g->inc[i][j]->arcs[a].pcom[ii].cout, g->inc[i][j]->arcs[a].pcom[ii].capa, g->inc[i][j]->arcs[a].pcom[ii].flot);
						}
					}
				}
			}
		}
	}

	printf("--- Listes des voisins sortants : ---\n");
	for(int i = 0; i < g->n; i++)
	{
		printf("%d : ", i+1);
		for(int j = 0; j < g->sortants[i].nbSort; j++)
		{
			printf("%d ", g->sortants[i].sort[j]+1);
		}
		printf("\n");
	}
}

void afficher_mut(Graphe *g)
{
	printf("----- ARCS MUTUALISES : -----\n");

	{
		for(int i = 0; i < g->n; i++)
		{
			for(int j = 0; j < g->n; j++)
			{
				if(g->inc[i][j] != NULL)
				{
					for(int a = 0; a < g->inc[i][j]->nbArcs; a++)
					{
						if(g->inc[i][j]->arcs[a].mut >= 0)
						{
							printf("arc %d (%d->%d) : ", g->inc[i][j]->arcs[a].nomArc, i+1, j+1);
							printf("%d", g->inc[i][j]->arcs[a].flotTotal);
							printf("/%d", g->inc[i][j]->arcs[a].mut);
							if(g->inc[i][j]->arcs[a].mut < g->inc[i][j]->arcs[a].flotTotal) printf(" (depassement)");
							printf("\n");
						}
					}
				}
			}
		}
	}
}



