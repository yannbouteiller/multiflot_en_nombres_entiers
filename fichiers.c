#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "constantes.h"
#include "fichiers.h"

int charger_probleme(Probleme *p, int instance)
{
	char CHAINE_ARC[LONGUEUR_CHAINE];
	if(sprintf(CHAINE_ARC, "instances/%d/%d.arc", instance, instance) < 0)
	{
		printf("ERREUR fichiers\n");
		exit(EXIT_FAILURE);
	}
	char CHAINE_MUT[LONGUEUR_CHAINE];
	if(sprintf(CHAINE_MUT, "instances/%d/%d.mut", instance, instance) < 0)
	{
		printf("ERREUR fichiers\n");
		exit(EXIT_FAILURE);
	}
	char CHAINE_NOD[LONGUEUR_CHAINE];
	if(sprintf(CHAINE_NOD, "instances/%d/%d.nod", instance, instance) < 0)
	{
		printf("ERREUR fichiers\n");
		exit(EXIT_FAILURE);
	}
	char CHAINE_SUP[LONGUEUR_CHAINE];
	if(sprintf(CHAINE_SUP, "instances/%d/%d.sup", instance, instance) < 0)	
	{
		printf("ERREUR fichiers\n");
		exit(EXIT_FAILURE);
	}
	
	FILE* fic = NULL;

	fic = fopen(CHAINE_NOD, "r"); // ouverture du .nod pour récupérer les paramètres du problème
	if (fic != NULL)
	{
		fscanf(fic, "%d", &p->com);
		fscanf(fic, "%d", &p->n);
		fscanf(fic, "%d", &p->a);
		fscanf(fic, "%d", &p->ac);

		fclose(fic);
	}
	else
	{
		printf("ERREUR LECTURE FICHIER NOD\n");
		return 0;
	}

	p->mut = calloc(p->ac, sizeof(SMut));

	fic = fopen(CHAINE_MUT, "r"); // ouverture du .mut
	if (fic != NULL)
	{
		for(int i = 0; i < p->ac; i++)
		{
			double tmp;
			fscanf(fic, "%d %lf", &p->mut[i].pm, &tmp);
			p->mut[i].cm = floor(tmp);
		}
		p->tailleMut = p->ac;

		fclose(fic);
	}
	else
	{
		printf("ERREUR LECTURE FICHIER MUT\n");
		return 0;
	}

	fic = fopen(CHAINE_SUP, "r"); // ouverture du .sup
	if (fic != NULL)
	{
		int nbLignes = 0;
		nbLignes = compter_lignes(fic);

		p->sup = calloc(nbLignes, sizeof(SSup));

		for(int i = 0; i < nbLignes; i++)
		{
			fscanf(fic, "%d %d %d", &p->sup[i].n, &p->sup[i].com, &p->sup[i].sup);
		}
		p->tailleSup = nbLignes;

		fclose(fic);
	}
	else
	{
		printf("ERREUR LECTURE FICHIER MUT\n");
		return 0;
	}

	fic = fopen(CHAINE_ARC, "r"); // ouverture du .arc
	if (fic != NULL)
	{
		int nbLignes = 0;
		nbLignes = compter_lignes(fic);

		p->arc = calloc(nbLignes, sizeof(SArc));

		for(int i = 0; i < nbLignes; i++)
		{
			double tmp;
			fscanf(fic, "%d %d %d %d %lf %lf %d", &p->arc[i].arc, &p->arc[i].i, &p->arc[i].j, &p->arc[i].com, &p->arc[i].cout, &tmp, &p->arc[i].pm);
			p->arc[i].capa = floor(tmp); // problème en nombres entiers -> capacités entières
		}
		p->tailleArc = nbLignes;

		fclose(fic);
	}
	else
	{
		printf("ERREUR LECTURE FICHIER ARC\n");
		return 0;
	}

	return 1;
}

int compter_lignes(FILE* fic)
{
	if(fic == NULL) return -1;
	else
	{
		int c;
		int nbLignes = 0;
		int cc = '\0';

		fseek(fic, 0, SEEK_SET);

		while((c=fgetc(fic)) != EOF)
		{
			if(c=='\n') nbLignes++;

			cc = c;
		}

		if(cc != '\n') nbLignes++;

		fseek(fic, 0, SEEK_SET);

		return nbLignes;
	}
}


