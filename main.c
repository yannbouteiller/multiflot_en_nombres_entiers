#include <stdio.h>
#include <stdlib.h>

#include "constantes.h"
#include "fichiers.h"
#include "graphe.h"
#include "algorithmes.h"

int main(int argc, char* argv[])
{
	Probleme p;
	
	int choix = 0;
	
	printf("\nMULTIFLOT EN NOMBRES ENTIERS - Yann Bouteiller\n\n");
	printf("Entrer le numero de l'instance a traiter (1 - 9) : ");
	scanf("%d", &choix);
	if(choix < 1 || choix > 9) choix = 1;
	
	charger_probleme(&p,choix);
	afficher_probleme(&p);
	Graphe g;
	generer_graphe(&p, &g);
	afficher_graphe(&g);

	LiArcs h;
	allouer_graphe_ecart(&h, &g);
	
	int continuer = 1;
	while(continuer == 1)
	{
		int c; // flush stdin
		do
		{
			c = getchar();
		} while (c != EOF && c != '\n');
		
		int I;
		double alpha;
		
		printf("Entrer le nombre maximal d'iterations infructueuses I : ");
		scanf("%d", &I);
		if(I < 1) I = 1;
		printf("Entrer le parametre alpha : ");
		scanf("%lf", &alpha);
		if(alpha < 0) alpha = 1;
		else if(alpha > 2) alpha = 2;
		
		sous_gradients(&g, &h, I, alpha);
		
		printf("Afficher le graphe obtenu ? (1 : oui / 0 : non) : ");
		scanf("%d", &choix);
		if(choix == 1)
		{
			afficher_graphe(&g);
			afficher_mut(&g);
		}
		
		printf("Continuer ? (1 : oui / 0 : non) : ");
		scanf("%d", &choix);
		if(choix == 0) continuer = 0;
		else reinitialiser_flots(&g);
	}
		
	detruire_graphe_ecart(&h, &g);

	return 0;
}


