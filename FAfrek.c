/*
 * afrek.c -- programma om een leningen afschrift te produceren
 */

#define	JAN		0
#define	FEB		1
#define	DEC		11
#define	NAMSIZ	39

#include	<stdio.h>
#include	<errno.h>

main()
{
	static	char	maand[12][4] = {"jan", "feb", "mar", "apr", "mei",
		"jun", "jul", "aug", "sep", "okt", "nov", "dec"};
	char	c, code, eindemaand, naam[NAMSIZ+1];
	static	short	laatstedag[12] = {31, 28, 31, 30, 31, 30, 31, 31,
							30, 31, 30, 31};
	short	dag, i, jaar, mnd;
	float	percentage = 0.5;
	float	centrente, mutatie, rente, rentesom, saldo, uitkering;
	FILE	*uit;

 	if ((uit = fopen("uitvoer", "w")) == NULL) {
		printf("File uitvoer can't be opened due to error %d\n",
			errno);
		exit(-1);
	}

	printf("Type naam:");
	i = 0;
	while ((c = getchar()) != '\n') {
		if (i < NAMSIZ) {
			naam[i++] = c;
		}
	}
	naam[i] = '\0';
 	printf("Type jaar:");
 	scanf("%d", &jaar);
 	if ((jaar % 4) == 0) {
 		laatstedag[FEB] = 29;
	}
 	printf("Type saldo per 1 jan. :");
 	scanf("%f", &saldo);
 	fprintf(uit, "W. K. M. Storbeck.\nBakkersteeg 11\nAlmen (GLD.)\n");
 	fprintf(uit, "______________________________\n");
 	fprintf(uit, "Jaarafrekening van de lening verstrekt aan ");
	fprintf(uit, "%s", naam);
 	fprintf(uit, " over %d\n", jaar);
 	fprintf(uit, "\t\t\t\t        debet\t       credit\n");
 	printf("Type steeds per maand de mutaties in.\n");
	printf(" <code> <datumno> <bedrag>\n");
 	printf("De codes zijn u(itkering), o(ntvangst), en r(ente).\n");

 	rentesom = 0.0;

 	for (mnd = JAN; mnd <= DEC; mnd++) {
 		fprintf(uit, " 1\t%s\tSaldo\t\t%10.2f\n", maand[mnd], saldo);
		printf("Mutaties %3s:\n", maand[mnd]);
		eindemaand = FALSE;
		while (eindemaand == FALSE) {
			while (getchar() != '\n') ;
			scanf("%c", &code);
			if (code == 'u' || code == 'o') {
				scanf("%d%f", &dag, &mutatie);
			}
			switch (code) {
			case 'r':
				centrente = (int)(saldo * percentage);
				rente = centrente * 0.01;
				saldo = saldo + rente + uitkering;
				rentesom = rentesom + rente;
				fprintf(uit, "%2d\t%s\tRente\t\t%10.2f\n",
					laatstedag[mnd], maand[mnd], rente);
				fprintf(uit, "\t\t\t\t\t___________    ____________\n");
				eindemaand = TRUE;
				break;
			case 'o': 
				saldo = saldo - mutatie;
				fprintf(uit, "%2d\t%3s\tOntvangen\t\t\t\t\t%10.2f\n",
					dag, maand[mnd], mutatie);
				break;
			case 'u': 
				uitkering = uitkering + mutatie;
				fprintf(uit, "%2d\t%3s\tUitkering\t%10.2f\n", dag,
					maand[mnd], mutatie);
				break;
			default:
				printf("Code is fout: %c!\n", code);
			}
		}
	}
 	fprintf(uit, " 1\tjan %2d  Saldo\t\t%10.2f\n", jaar % 100, saldo);
	fprintf(uit, "Totaal werd aan rente betaald: %10.2f\n", rentesom);
}

