/* fuitvoer.c */

/*	fuitvoer maakt deposito-afschriften per deelnemer op basis van:			*/
/*		namen.dta				een file met namen van deelnemers en relaties	*/
/*		form3.dta				een model van het uitvoer-formulier					*/
/*		basis/bas<jjjjm>.dta	basisfiles van begin en einde van de periode		*/
/*	verondersteld is dat de eerste twee files in de current directory staan */
/* en de basis-files in subdirectory "basis"											*/

/*	inhoud en formaat van de uitvoer kunnen door wijziging van de '#'codes	*/
/* in "form3.dta" gewijzigd worden.														*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "FBasis.h"

		/* plaatsbepaling op de uitvoerpagina                                */
char	 pagina[5000]			/* inhoud pagina uitvoer (regelbreedte 72)      */
		,paginb[5000]			/* inhoud pagina uitvoer (regelbreedte 72)      */
		,reg[80]; 				/* ontwerp regel of deel daarvan                */
int		 k,k0						/* positie op de pagina                         */
		,pagesize				/* totale omvang pagina in characters           */
		,lines					/* totaal aantal regels op de pagina            */
		,r,regel;				/* regel op de pagina                           */

		/* de cijfers 0 en 1 op het einde van namen van variabelen refereren */
		/* naar referentiedatum (begin van de periode)                       */
		/* en   actuele datum   (einde van de periode)                       */

		/* Financiele wijzigingen															*/
double	 splcor				/* factor bij splitsing (=Inleg1 /Inleg0)			*/
			,valcor				/* factor bij valutawyz (=Valut1/Valut0)			*/
			,totcor;				/*	product van deze factoren							*/

int		 Jaar0,Jaar1			/* Jaartal													*/
		,Maand0,Maand1;		/* Maand														*/

		/* FOEF parameters																	*/
long	 Inleg0,Inleg1			/* Inleg per maand										*/
		,Rente0,Rente1			/* Interne berekende rente op rekening-courant	*/
		,Valut0,Valut1			/* Valut t.o.v. de gulden								*/
		,DNFactor0,DNFactor1	/* Splitsingsfactor sinds begin */
		,Vermogen0,Vermogen1;/* Vermogen	FOEF											*/

		/* gegevens per deelneming															*/
long	 WdvPD0,WdvPD1			/* Verkoopwaarde van een deelneming					*/
		,InlPD0,InlPD1			/* Totale inleg per deelneming						*/
		,ExwPD0,ExwPD1			/* Totale externe winst per deelneming				*/
		,WinPD0,WinPD1;		/* Totale winst per deelneming						*/

		/* totalen gegevens deelnemers													*/
long	 Deb_T0,Deb_T1			/* Totaal debiteuren										*/
		,Cre_T0,Cre_T1 		/* Totaal crediteuren									*/
		,Dng_T0,Dng_T1			/* aantal deelnemingen totaal                   */
		,Bet_T0,Bet_T1			/* cumulatieve betalingen en overboekingen tot  */
		,Dep_T0,Dep_T1			/* totalen deposito's deelnemers						*/
		,Gdp_T0,Gdp_T1			/* gemiddelde deposito deelnemers					*/
		,Exw_T0,Exw_T1;		/* cumulatieve externe winst totaal					*/



int	LeesBasis()
{
	int	 Index;					/* Volgnummer deelnemer of relatie					*/
	zdr	 lDnrPtr;
	zrl	lRelPtr;
	char	Record0[120];
	char	Record1[120];
	char	BasN1[60],BasN0[60];	/* naam basisfiles                              */
	FILE	 *BasF1,*BasF0;			/* basisfiles                                   */
	
	sprintf(BasN0,"basis/bas%4d%c.dta", Jaar0, '`'+Maand0);
	BasF0 = fopen(BasN0,"r");
	if (BasF0 == NULL)
	{
		printf("De eerste basisfile: %s is helaas niet aanwezig\n",BasN0);
		printf("Dit programma wordt nu afgebroken\n");
		exit(1);
	}

	sprintf(BasN1,"basis/bas%4d%c.dta" ,Jaar1,'`'+Maand1);
	BasF1=fopen(BasN1,"r");
	if (BasF1==NULL)
	{
		printf("De tweede basisfile: %s is helaas niet aanwezig\n",BasN1);
		printf("Dit programma wordt nu afgebroken\n");
		exit(1);
	}
	
	
	/* lezen gegevens eerste twee regels file 0 en 1	(eerste regel bevat labels)	
	   bijvoorbeeld:
	 jaar mn inlm dnspl valuta verkwaarde  inleg/dln   vermogen  exwin/dln  winst/dln debiteuren
	 1970  7  200    1 1000000        198        200     100475          0         -2       6000
	 */
	fgets(Record0,115,BasF0); fgets(Record0,115,BasF0);
	fgets(Record1,115,BasF1); fgets(Record1,115,BasF1);
	Jaar0  =atoi(Record0   );
	Jaar1  =atoi(Record1   );
	Maand0 =atoi(Record0+ 4);
	Maand1 =atoi(Record1+ 4);
	DNFactor0 =atoi(Record0+12);
	DNFactor1 =atoi(Record1+12);
	Valut0 =atol(Record0+17);
	Valut1 =atol(Record1+17);

	valcor = (double)Valut1 / Valut0;
	Inleg0 = rondaf(atoi(Record0+ 7)*valcor);
	Inleg1 = atoi(Record1+ 7);
	/* Berent: dit was splcor = (double)Inleg1 / Inleg0; 
	   maar dat kan niet, want de inleg per dn kan apart worden ingesteld.. */
	splcor = (double)DNFactor0 / DNFactor1; 
	totcor = valcor * splcor; 

	WdvPD0 =rondaf(atol(Record0+25)*totcor);
	WdvPD1 =atol(Record1+25);
	InlPD0 =rondaf(atol(Record0+36)*totcor);
	InlPD1 =atol(Record1+36);
	Vermogen0=rondaf(atol(Record0+47)*valcor);
	Vermogen1=atol(Record1+47);
	ExwPD0 =rondaf(atol(Record0+58)*totcor);
	ExwPD1 =atol(Record1+58);
	WinPD0 =rondaf(atol(Record0+69)*totcor);
	WinPD1 =atol(Record1+69);
	Deb_T0 =rondaf(atol(Record0+80)*valcor);
	Deb_T1 =atol(Record1+80);
	
	/* lezen gegevens tweede twee regels file 0 en 1	(eerste regel bevat labels)
	 bijvoorbeeld:
	 rente   aantal crediteuren  deposito    betaald  gem depos  ext winst
	 600       500      14000     107000     108000     107000          0
	 */
	fgets(Record0,115,BasF0); 
	fgets(Record0,115,BasF0);
	fgets(Record1,115,BasF1); 
	fgets(Record1,115,BasF1);

	Rente0 =atoi(Record0);
	Rente1 =atoi(Record1);
	Dng_T0 =rondaf(atoi(Record0+ 7)/splcor);
	Dng_T1 =atoi(Record1+ 7);
	Cre_T0 =rondaf(atol(Record0+14)*valcor);
	Cre_T1 =atol(Record1+14);
	Dep_T0 =rondaf(atol(Record0+25)*valcor);
	Dep_T1 =atol(Record1+25);
	Bet_T0 =rondaf(atol(Record0+36)*valcor);
	Bet_T1 =atol(Record1+36);
	Gdp_T0 =rondaf(atol(Record0+47)*valcor);
	Gdp_T1 =atol(Record1+47);
	Exw_T0 =rondaf(atol(Record0+58)*valcor);
	Exw_T1 =atol(Record1+58);
	
	/* lezen labels deelnemers
	   bijvoorbeeld:
	 deelnr  aantal    rek-crt   deposito    betaald  gem depos  ext winst  winst dln  mutkost rc rente   overboek
	 */
	fgets(Record0,115,BasF0);
	fgets(Record1,115,BasF1);
	/* lezen gegevens deelnemers
	 bijvoorbeeld:
	 001 kn      10          0       1980       2000       1980          0        -20        0        0          0
	 */
	while (fgets(Record0,115,BasF0)!=NULL && Record0[0]<58) /* stop als eerste karakter niet numeriek is: volgende labelregel */
	{
		Index=atoi(Record0);
		AddDnr(Index, &lDnrPtr);
		lDnrPtr->Dng=rondaf(atoi(Record0+7)/splcor);
		lDnrPtr->Rcr=rondaf(atol(Record0+14)*valcor);
		lDnrPtr->Dep=rondaf(atol(Record0+25)*valcor);
		lDnrPtr->Bet=rondaf(atol(Record0+36)*valcor);
		lDnrPtr->Gdp=rondaf(atol(Record0+47)*valcor);
		lDnrPtr->Exw=rondaf(atol(Record0+58)*valcor);
		lDnrPtr->Wid=rondaf(atol(Record0+69)*valcor);
		lDnrPtr->Muk=rondaf(atol(Record0+80)*valcor);
		lDnrPtr->Ren=rondaf(atol(Record0+89)*valcor);
		lDnrPtr->Ove=rondaf(atol(Record0+98)*valcor);
	}
	while (fgets(Record1,115,BasF1)!=NULL && Record1[0]<58) /* stop als eerste karakter niet numeriek is: volgende labelregel */
	{
		Index=atoi(Record1);
		AddDnr(Index, &lDnrPtr);
		lDnrPtr->Dng1=atoi(Record1+7);
		lDnrPtr->Rcr1=atol(Record1+14);
		lDnrPtr->Dep1=atol(Record1+25);
		lDnrPtr->Bet1=atol(Record1+36);
		lDnrPtr->Gdp1=atol(Record1+47);
		lDnrPtr->Exw1=atol(Record1+58);
		lDnrPtr->Wid1=atol(Record1+69);
		lDnrPtr->Muk1=atol(Record1+80);
		lDnrPtr->Ren1=atol(Record1+89);
		lDnrPtr->Ove1=atol(Record1+98);
	}

	/* lezen gegevens relaties
	 bijvoorbeeld:
	 601 abn      0      56425      56425      56425      56425
	 */
	while (fgets(Record0,115,BasF0)!=NULL && Record0[0]<58) /* stop als eerste karakter niet numeriek is: volgende labelregel */
	{
		Index=atoi(Record0);
		AddRel(Index, &lRelPtr);
		lRelPtr->Eff=rondaf(atol(Record0+ 7)*valcor);
		lRelPtr->Teg=rondaf(atol(Record0+14)*valcor);
		lRelPtr->Sld=rondaf(atol(Record0+25)*valcor);
		lRelPtr->Sto=rondaf(atol(Record0+36)*valcor);
		lRelPtr->Gsl=rondaf(atol(Record0+47)*valcor);
	}
	while (fgets(Record1,115,BasF1)!=NULL && Record1[0]<58) /* stop als eerste karakter niet numeriek is: volgende labelregel */
	{
		Index=atoi(Record0);
		AddRel(Index, &lRelPtr);
		lRelPtr->Eff1=atol(Record1+ 7);
		lRelPtr->Teg1=atol(Record1+14);
		lRelPtr->Sld1=atol(Record1+25);
		lRelPtr->Sto1=atol(Record1+36);
		lRelPtr->Gsl1=atol(Record1+47);
	}
	/* lezen gegevens groepen
	 bijvoorbeeld:
	 500 FON      0          0    -543751     970451     -19491
	 600 BAN      0          0    3550948    2958155     931891
	 700 EFF      0          0    6681848    3416058    3908018
	 800 OBL      0          0    4834928     551348    2116949
	 900 REL      0          0   14523973    7896012    6937367
	 */
	fgets(Record0,115,BasF0);
	FuT.Eff=rondaf(atol(Record0+ 7)*valcor);
	FuT.Teg=rondaf(atol(Record0+14)*valcor);
	FuT.Sld=rondaf(atol(Record0+25)*valcor);
	FuT.Sto=rondaf(atol(Record0+36)*valcor);
	FuT.Gsl=rondaf(atol(Record0+47)*valcor);
	fgets(Record0,115,BasF0);
	BaT.Eff=rondaf(atol(Record0+ 7)*valcor);
	BaT.Teg=rondaf(atol(Record0+14)*valcor);
	BaT.Sld=rondaf(atol(Record0+25)*valcor);
	BaT.Sto=rondaf(atol(Record0+36)*valcor);
	BaT.Gsl=rondaf(atol(Record0+47)*valcor);
	fgets(Record0,115,BasF0);
	EfT.Eff=rondaf(atol(Record0+ 7)*valcor);
	EfT.Teg=rondaf(atol(Record0+14)*valcor);
	EfT.Sld=rondaf(atol(Record0+25)*valcor);
	EfT.Sto=rondaf(atol(Record0+36)*valcor);
	EfT.Gsl=rondaf(atol(Record0+47)*valcor);
	fgets(Record0,115,BasF0);
	ObT.Eff=rondaf(atol(Record0+ 7)*valcor);
	ObT.Teg=rondaf(atol(Record0+14)*valcor);
	ObT.Sld=rondaf(atol(Record0+25)*valcor);
	ObT.Sto=rondaf(atol(Record0+36)*valcor);
	ObT.Gsl=rondaf(atol(Record0+47)*valcor);
	fgets(Record0,115,BasF0);
	ReT.Eff=rondaf(atol(Record0+ 7)*valcor);
	ReT.Teg=rondaf(atol(Record0+14)*valcor);
	ReT.Sld=rondaf(atol(Record0+25)*valcor);
	ReT.Sto=rondaf(atol(Record0+36)*valcor);
	ReT.Gsl=rondaf(atol(Record0+47)*valcor);
	fgets(Record1,115,BasF1);
	FuT.Eff1=atol(Record1+ 7);
	FuT.Teg1=atol(Record1+14);
	FuT.Sld1=atol(Record1+25);
	FuT.Sto1=atol(Record1+36);
	FuT.Gsl1=atol(Record1+47);
	fgets(Record1,115,BasF1);
	BaT.Eff1=atol(Record1+ 7);
	BaT.Teg1=atol(Record1+14);
	BaT.Sld1=atol(Record1+25);
	BaT.Sto1=atol(Record1+36);
	BaT.Gsl1=atol(Record1+47);
	fgets(Record1,115,BasF1);
	EfT.Eff1=atol(Record1+ 7);
	EfT.Teg1=atol(Record1+14);
	EfT.Sld1=atol(Record1+25);
	EfT.Sto1=atol(Record1+36);
	EfT.Gsl1=atol(Record1+47);
	fgets(Record1,115,BasF1);
	ObT.Eff1=atol(Record1+ 7);
	ObT.Teg1=atol(Record1+14);
	ObT.Sld1=atol(Record1+25);
	ObT.Sto1=atol(Record1+36);
	ObT.Gsl1=atol(Record1+47);
	fgets(Record1,115,BasF1);
	ReT.Eff1=atol(Record1+ 7);
	ReT.Teg1=atol(Record1+14);
	ReT.Sld1=atol(Record1+25);
	ReT.Sto1=atol(Record1+36);
	ReT.Gsl1=atol(Record1+47);
	
	fclose(BasF0);
	fclose(BasF1);
	return 0;
}

	/* lezen van de model opmaak                                            */
int	LeesForm()
{
	char	 Record0[120];
	FILE	 *ModelF;					/* modelfile                                    */
	
	ModelF=fopen("form3.dta","r");
	if (ModelF==NULL)
	{
		printf("De formulier file: %s is helaas niet aanwezig\n", "form3.dta");
		printf("Dit programma wordt nu afgebroken\n");
		exit(1);
	}

	regel=0;
	while (fgets(Record0,80,ModelF)!=NULL)
	{
		sprintf(pagina+regel*72,"%.72s",Record0+1);
		regel++;
	}
	fclose(ModelF);
	pagesize=regel*72;
	
	return 0;
}

char	*Woord(int gn,char *ptr)		/* format voor een naam									*/
{
	sprintf(reg,"%.*s",gn,ptr);
	return reg;
}

char	*Somma(long f)			/* format voor een geldsom								*/
{
	sprintf(reg,"%10.2f",(float) f/100.);
	return reg;
}

char	*Datum(int y,int m)	/* format voor een datum (jaar en maand)			*/
{
	sprintf(reg,"%4d-%02d",y,m);
	return reg;
}

char	*Getal(int gn,long gt)	/* format voor een getal							*/
{
	sprintf(reg,"%*ld",gn,gt);
	return reg;
}

char	*Procent(int gn,long gt)	/* format voor een percentage					*/
{
	sprintf(reg,"%*ld%%",gn,gt);
	return reg;
}

		/* gegevens invoegen op de juiste plaatsen									*/
void	VoegIn(const char *doel,char *tekst)
{
	k=0;
	while (k<pagesize)
	{
		if (pagina[k]=='#' && strncmp(pagina+k+1,doel,strlen(doel))==0)
		{	
			int j=0; 
			
			while (tekst[j]>=32)  {	
				pagina[k+j]=tekst[j]; 
				j++; 
			}  
		}
		k++;
	}
}

		/* opstellen van deposito-afschriften per deelnemer						*/
void	Afschriften()
{
	int j;
	
			/* percentages debiteuren, contanten, aandelen en obligaties      */
	long	pd,pb,pe,po;
			/* hulpvariabelen																	*/
	long	f1,f2,f3;
	zrl		lRelPtr;
	zdr		lDnrPtr;
	char	 DepaN[60];				/* naam uitvoerfile deposito-afschriften        */
	char	 ScriptN[60];				/* naam uitvoerfile email-script        */
	FILE	 *DepaF;					/* uitvoerfile deposito-afschriften             */
	FILE	 *ScriptF;
	
	
	
	sprintf(ScriptN,"uitvoer/ Script %4d-%02d.txt",Jaar1,Maand1);
	ScriptF=fopen(ScriptN,"w");

	
	LeesForm();
	VoegIn("Maand0",Datum(Jaar0,Maand0));
	VoegIn("Maand1",Datum(Jaar1,Maand1));
	/* percentages beleggingsobjecten berekenen                             */
	pd=(100.*Deb_T1/Vermogen1+.5);
	pb=(100.*BaT.Sld1/Vermogen1+.5);
	pe=(100.*EfT.Sld1/Vermogen1+.5);
	po=(100.*ObT.Sld1/Vermogen1+.5);

	/* paragraaf A schrijven                                                */
	VoegIn("d",Procent(2,pd));
	VoegIn("b",Procent(2,pb));
	VoegIn("e",Procent(2,pe));
	VoegIn("o",Procent(2,po));
	f1=f2=0;
	VoegIn("DebT",Somma(Deb_T1));		f1+=Deb_T1;
	VoegIn("CreT",Somma(Cre_T1));		f2+=Cre_T1;
	VoegIn("BanT",Somma(BaT.Sld1));	f1+=BaT.Sld1;
	VoegIn("FonT",Somma(-FuT.Sld1));	f2-=FuT.Sld1;
	VoegIn("EffT",Somma(EfT.Sld1));	f1+=EfT.Sld1;
	VoegIn("OblT",Somma(ObT.Sld1));	f1+=ObT.Sld1;
	VoegIn("VerT",Somma(Vermogen1));	f2+=Vermogen1;

	/* regels voor informatie effecten invoegen                             */
	regel=6;
	lRelPtr=zRe0;
	while (lRelPtr)
	{
		if (lRelPtr->Typ==7 && lRelPtr->Eff1)
		{
			pagesize+=72; k=pagesize; k0=regel*72;
			while (k>=k0+72) { pagina[k]=pagina[k-72]; k--; }
			sprintf(reg,"    %5ld %.3s %10.2f                                                "
			,lRelPtr->Eff1,lRelPtr->Rnm,(float) lRelPtr->Sld1*.01);
			for (j=0;j<72;j++) pagina[regel*72+j]=reg[j];
			regel++;
		}
		lRelPtr=lRelPtr->vlg;
	}
	regel++;
	lRelPtr=zRe0;
	while (lRelPtr)
	{
		if (lRelPtr->Typ==8 && lRelPtr->Eff1)
		{
			pagesize+=72; k=pagesize; k0=regel*72;
			while (k>=k0+72) { pagina[k]=pagina[k-72]; k--; }
			sprintf(reg,"    %5ld %.3s %10.2f                                                "
			,lRelPtr->Eff1,lRelPtr->Rnm,(float) lRelPtr->Sld1*.01);
			for (j=0;j<72;j++) pagina[regel*72+j]=reg[j];
			regel++;
		}
		lRelPtr=lRelPtr->vlg;
	}
	VoegIn("LinT",Somma(f1));
	VoegIn("RecT",Somma(f2));

	/* paragraaf B schrijven                                                */
	VoegIn("DnT1",Getal(5,Dng_T1));
	VoegIn("DnT0",Getal(5,Dng_T0));
	VoegIn("DnTv",Getal(5,Dng_T1-Dng_T0));
	VoegIn("WdvPD1",Somma(WdvPD1));
	VoegIn("WdvPD0",Somma(WdvPD0));
	VoegIn("WdvPDv",Somma(WdvPD1-WdvPD0));
	VoegIn("InlPD1",Somma(InlPD1));
	VoegIn("InlPD0",Somma(InlPD0));
	VoegIn("InlPDv",Somma(InlPD1-InlPD0));
	VoegIn("WinPD1",Somma(WinPD1));
	VoegIn("WinPD0",Somma(WinPD0));
	VoegIn("WinPDv",Somma(WinPD1-WinPD0));
	VoegIn("ExwPD1",Somma(ExwPD1));
	VoegIn("ExwPD0",Somma(ExwPD0));
	VoegIn("ExwPDv",Somma(ExwPD1-ExwPD0));

	/* paragraaf C schrijven                                                */
	VoegIn("DepT1",Somma(Dep_T1));
	VoegIn("DepT0",Somma(Dep_T0));
	VoegIn("DepTv",Somma(Dep_T1-Dep_T0));
	VoegIn("InlT1",Somma(Bet_T1));
	VoegIn("InlT0",Somma(Bet_T0));
	VoegIn("InlTv",Somma(Bet_T1-Bet_T0));
	VoegIn("WinT1",Somma(f1=Dep_T1-Bet_T1));
	VoegIn("WinT0",Somma(f2=Dep_T0-Bet_T0));
	VoegIn("WinTv",Somma(f1-f2));
	VoegIn("ExwT1",Somma(Exw_T1));
	VoegIn("ExwT0",Somma(Exw_T0));
	VoegIn("ExwTv",Somma(Exw_T1-Exw_T0));

	/* individuele gedeelte schrijven                                       */
	lines=pagesize/72;
	for (j=0;j<pagesize;j++) paginb[j]=pagina[j];
	lDnrPtr=zDn0;
	while (lDnrPtr)
	{
		if (lDnrPtr->Dng!=0 || lDnrPtr->Rcr!=0 || lDnrPtr->Dng1!=0 || lDnrPtr->Rcr1!=0)
		{
			char lFileNaam[40];
			
			sprintf(lFileNaam,"%03d%3s%4d-%02d.txt",lDnrPtr->Idx,lDnrPtr->Dnm,Jaar1,Maand1);
			fprintf(ScriptF, "%s\t%s\r", lFileNaam, lDnrPtr->Email);
			
			sprintf(DepaN,"uitvoer/%s",lFileNaam);
			DepaF=fopen(DepaN,"w");

			for (j=0;j<pagesize;j++) pagina[j]=paginb[j];
			VoegIn("Naam",Woord(20,lDnrPtr->Naam));
			VoegIn("Dng1",Getal(5,lDnrPtr->Dng1));
			VoegIn("Dng0",Getal(5,lDnrPtr->Dng));
			VoegIn("Dngv",Getal(5,lDnrPtr->Dng1-lDnrPtr->Dng));
			VoegIn("Wdv1",Somma(f1=lDnrPtr->Dng1*WdvPD1));
			VoegIn("Wdv0",Somma(f2=lDnrPtr->Dng*WdvPD0));
			VoegIn("Wdvv",Somma(f1-f2));
			VoegIn("Rcr1",Somma(lDnrPtr->Rcr1));
			VoegIn("Rcr0",Somma(lDnrPtr->Rcr));
			VoegIn("Rcrv",Somma(lDnrPtr->Rcr1-lDnrPtr->Rcr));
			VoegIn("Dep1",Somma(lDnrPtr->Dep1));
			VoegIn("Dep0",Somma(lDnrPtr->Dep));
			VoegIn("Depv",Somma(f1=lDnrPtr->Dep1-lDnrPtr->Dep));

			VoegIn("Ovev",Somma(f2=lDnrPtr->Ove1-lDnrPtr->Ove));
			VoegIn("Betv",Somma(f3=lDnrPtr->Bet1-lDnrPtr->Bet));
			VoegIn("Hulv",Somma(f3-f2));

			VoegIn("Winv",Somma(lDnrPtr->Wid1-lDnrPtr->Wid));
			VoegIn("Mukv",Somma(lDnrPtr->Muk1-lDnrPtr->Muk));
			VoegIn("Renv",Somma(lDnrPtr->Ren1-lDnrPtr->Ren));

			VoegIn("Netv",Somma(f1-f3));

			for (r=0;r<lines;r++) fprintf(DepaF,"%.72s\n",pagina+r*72);
			
			fclose(DepaF);
		}
		lDnrPtr=lDnrPtr->vlg;
	}
	fclose(ScriptF);
}

		/* opdrachtformulering																*/
int	Dialoog()
{
	char	iq;
	int	klaar;
//	clrscr();
	klaar=0;
	while (!klaar)
	{
		printf("Welke periode ?\n");
		printf("  Geef jaar en maand (jjjj mm) van het vorige Afschriften\n");
		Jaar0=atoi(gets(reg)); Maand0=atoi(reg+5);
		printf("\n  Geef jaar en maand (jjjj mm) van het nieuw Afschriften\n");
		Jaar1=atoi(gets(reg)); Maand1=atoi(reg+5);
		printf("\n  Er worden Afschriften gemaakt van:\n");
		printf("\n    %4d-%02d   tot   %4d-%02d\n"
		,Jaar0,Maand0,Jaar1,Maand1);
		printf("\nOK ? (geef j voor ja *** n voor nee *** s om te stoppen\n");
		iq=getchar();
		if (iq=='j' || iq=='J') klaar=1;
		if (iq=='s' || iq=='S') return 0;
	}
	printf("\n"); return 1;
}


int main(int argc, char *argv[])
{
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
		if ((Jaar0 > Jaar1) ||
			(Jaar0 == Jaar1 && Maand0 >= Maand1)) {
				printf("Geen geldige periode\n");
				exit -1;
		}
		
		LeesNamen();
		LeesBasis();
		Afschriften();
	}
#else ArgsVersion	
	if (Dialoog()) {
		LeesNamen();
		LeesBasis();
		Afschriften();
	}
#endif ArgsVersion
	return 0;
}
