/* fmutatie.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include "FBasis.h"

			/* de volgende variabelen worden ingelezen uit de muta-files		*/
char	 Mrec[120];		/* Muta-data record, bevat de volgende gegevens:		*/
int		Muco,				/* mutatie code                                       */
		Cred,				/* crediteur                                          */
		Debi,				/* debiteur                                           */
		Aant;			/* aantal | exponent                                  */
long	Bedrag;			/* bedrag | rente | inleg | valuta                    */
int		Mucv,				/* mutatie code vorig record									*/
		Pent;			/* beginjaar van de gezochte muta-file                */
		/* NB de muta-files zijn per 5 jaar gegroepeerd								*/

			/* algemene variabelen */
int		Start,			/* indicator of we binnen de periode zitten           */
		Jaar0,Maand0,	/* begin tijdstip													*/
		Jaar1,Maand1,	/* eind  tijdstip													*/
		Nieuw;			/* indicator nieuw gevonden deelnemersindex				*/
char	 Regel[180];

FILE	*MutaF,*UitvoerF;

int	Dialoog()
{
	char	iq;
	int	klaar;
//	clrscr();
	klaar=0;

	while (!klaar)
	{
		printf("Welke periode ?\n");
		printf("\n  Geef jaar en maand (jjjj mm) van de eerste lijst\n");
		Jaar0=atoi(gets(Regel)); Maand0=atoi(Regel+5);
		printf("\n  Geef jaar en maand (jjjj mm) van de laatste lijst\n");
		Jaar1=atoi(gets(Regel)); Maand1=atoi(Regel+5);
		printf("\n  Er worden lijsten gemaakt van:\n");
		printf("\n    %4d-%02d   tot en met   %4d-%02d\n"
		,Jaar0,Maand0,Jaar1,Maand1);
		printf("\nOK ? (geef j voor ja *** n voor nee *** s om te stoppen\n");
		iq=getchar();
		if (iq=='j' || iq=='J') klaar=1;
		if (iq=='s' || iq=='S') return 0;
	}
	printf("\n"); return 1;
}


void	Initialiseren()
{
	Pent=1970;
	Start=0;
	if (Jaar0<=1970 && Maand0<=6) Start=1;
}

void	Decoderen()
{
	zrl	zRel;
	zdr zDnrRel;
	zmut zMut;
	char *lMrec;
	
	lMrec = Mrec;
	
	if (*lMrec == '\r') lMrec++; /* Dit omdat op de Mac er soms een '\r' vooraan Mrec staat */
	
	if (strlen(lMrec) <= 25) { // lege regels mogen (korter dan 25 tekens), deze worden genegeerd
		return;
	}
	
	Muco=atoi(lMrec);

	/* mutaties kunnen zowel met lettercodes voor 
	   debiteur en crediteur als met cijfercodes worden aangegeven
	   */
	if (lMrec[6] >= '0' && lMrec[6] <= '9') {
		Debi = atoi(lMrec + 2);   // numerieke deb/cred code
	} else {
		ZoekDnrRelByName(lMrec + 2, (void**)&zDnrRel);
		Debi = zDnrRel ? zDnrRel->Idx : 0; // letter deb/cred code
	}
	if (lMrec[11] >= '0' && lMrec[11] <= '9') {
		Cred = atoi(lMrec + 7);   // numerieke deb/cred code
	} else {
		ZoekDnrRelByName(lMrec + 7, (void**)&zDnrRel);
		Cred = zDnrRel ? zDnrRel->Idx : 0; // letter deb/cred code
	}

	Bedrag=atol(lMrec+12);
	Aant=atoi(lMrec+23);
	
	if (Muco==10 && Debi==Jaar0 && Cred==Maand0) { 
		Start=1; 
	}
	
	if (!Start) {
		/* begin pas met verwerken van mutaties als de start van de periode
				   voorbij is gekomen */
		return;
	}
	
	if (Muco == 10) {
		/* begin maand, geef startregel weer met jaar/maand nummer */
		sprintf(Regel + 15,"%4d-%02d  bij                 af",Debi,Cred);
	} else if (Muco == 90) {
		/* einde maand, sluit af met jaar/maand nummer en lege regel */
		sprintf(Regel + 24,"%4d-%02d\n\n", Debi, Cred);
		if (Debi == Jaar1 && Cred == Maand1) {
			Start=0;
		}
	} else {
		ZoekMuta(Muco+900, &zMut); /* de namen van de mutaties staan vanaf nummer 900 in Namen.dta */
		sprintf(Regel,"%02d %-20s ",Muco, zMut ? zMut->Naam : "Onbekende mutatie!");

		ZoekDnrRel(Debi, (void **)&zRel);
		sprintf(Regel + 24,"%-20s   ", zRel ? zRel->Naam : " ");
		
		ZoekDnrRel(Cred, (void **)&zRel);
		sprintf(Regel + 44,"%-20s   ", zRel ? zRel->Naam : " ");
		
		if (Bedrag) 
			sprintf(Regel+64,"%7ld.%02ld ", Bedrag/100, Bedrag%100);
		else 
			sprintf(Regel+64,"           ");
		
		if (Aant) 
			sprintf(Regel+74,"%5d",Aant);
		else 
			sprintf(Regel+74,"      ");
	}
	fprintf(UitvoerF, "%-80s\n", Regel);

	return;
}

int main(int argc, char *argv[])
{
	char	 Mutanaam[120];
	int argsOk = 0;
	
#define ArgsVersion	
#ifdef ArgsVersion	
	if (argc != 5) {
		printf("Geef als argumenten de periode mee: jjjj mm jjjj mm\n");
		exit -1;
	} else {
		Jaar0=atoi(argv[1]);
		Maand0=atoi(argv[2]);
		
		Jaar1=atoi(argv[3]);
		Maand1=atoi(argv[4]);
		
		argsOk = 1;
	}
#else ArgsVersion	
	argsOk = Dialoog();
#endif ArgsVersion	

	
	if (argsOk)
	{
		LeesNamen();
//		clrscr();
		Initialiseren();
		UitvoerF = fopen("Mutaties.txt","w");
		while (1)
		{
			sprintf(Mutanaam,"%s%4d.dta","muta/muta",Pent);
			MutaF=fopen(Mutanaam,"r");
			if (MutaF==NULL) break;
			while (fgets(Mrec, 60, MutaF) != NULL) 
				Decoderen();
			fclose(MutaF);
			Pent += 5;
		}
		fclose(MutaF);
		fclose(UitvoerF);
	}
}
