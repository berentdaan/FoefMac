/*
 *  fbereken.c
 *  � 1970-2003 Fam. Daan
 *
 *  Questions and comments to:
 *       <mailto:berent@foef.nl>
 *       
 *  Historie: januari 2003 - Debugerk door Berent om het draaiend te krijgen op MacOS
 */


#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <math.h>

#include "FBasis.h"




			/* de volgende variabelen worden ingelezen uit de muta-files		*/
char	Mrec[120];		/* Muta-data record, bevat de volgende gegevens:		*/
int		Muco,				/* mutatie code                                       */
		Cred,				/* crediteur                                          */
		Debi,				/* debiteur                                           */
		Aant;			/* aantal | exponent                                  */
long	Bedrag;			/* bedrag | rente | inleg | valuta                    */
int		Mucv,				/* mutatie code vorig record									*/
		Pent;			/* beginjaar van de gezochte muta-file                */
			/* NB de muta-files zijn per 5 jaar gegroepeerd								*/

			/* algemene variabelen */
int		Mn,Jr,			/* maand, jaar                                        */
		NMn;				/* aantal maanden sedert 1-7-1970                     */
int		Errs,				/* aantal gevonden fouten in de muta-files            */
		Nieuw;			/* indicator nieuw gevonden deelnemersindex				*/

			/* FOEF parameters: inleg, rente, valuta; vermogen						*/
int		InlegM,  		/* basisbedrag dat per maand wordt ingelegd           */
		RenteI, 		/* interne rente per jaar in 0.01 procent             */
		DeelnFactor;	/* splitsingsfactor deelnemingen					*/
double	RenteJ,			/* exacte interne rente per jaar                      */
		RenteM; 		/* exacte interne rente per maand                     */
long	Valuta;			/* valuta in guldens *1000000                         */
double	ValutF;			/* valuta in guldens exact                            */

			/* FOEF totaal-gegevens															*/
long	Vermogen;		/* vermogen                                           */
long	Ex0T=0,			/* externe winst t/m vorige maand							*/
		CreTot,			/* totaal deelnemers-crediteuren								*/
		DebTot;			/* totaal deelnemers-debiteuren								*/

			/* gegevens per deelneming                                        */
long	ExwD,				/* cumulatieve externe winst per deelneming           */
		ExwV;			/* externe winst per deelneming in de laatste maand   */
long	WdeI,WdeA,		/* intrinsieke waarde en aankoopwaarde                */
		WdeV,Wd0V,		/* verkoopwaarde nu en vorige maand                   */
		InlD;			/* cumulatieve inleg per deelneming                   */


int	Fout(int Err,int mc,char *ErrText)
{
	static FILE *ErrorF = NULL;
	
	if (!ErrorF) {
		ErrorF=fopen("errorlog.dta","w");
	}
	
	switch(Err)
	{
	case 0:	fprintf(ErrorF,"Noot:   %4d-%2d %2d %3d %3d %s\n",Jr,Mn,mc,Debi,Cred,ErrText);
				break;
	case 1:	fprintf(ErrorF,"Pas op: %4d-%2d %2d %3d %3d %s\n",Jr,Mn,mc,Debi,Cred,ErrText);
				break;
	case 2:	fprintf(ErrorF,"Fout:   %4d-%2d %2d %3d %3d %s\n",Jr,Mn,mc,Debi,Cred,ErrText);
				break;
	case 3:	fprintf(ErrorF,"Negeer: %4d-%2d %2d %3d %3d %s\n",Jr,Mn,mc,Debi,Cred,ErrText);
				break;
	case 4:	fprintf(ErrorF,"Fataal: %4d-%2d %2d %3d %3d %s\n",Jr,Mn,mc,Debi,Cred,ErrText);
				printf("Fatale fout: het programma wordt beeindigd\n");
				fclose(ErrorF); 
				exit(1);
	}
	Errs++;
	if (Errs>100)
	{
		printf("Te veel fouten: het programma wordt beeindigd\n");
		fclose(ErrorF);
		exit(-1);
   }
	return Err;
}



void	Initialiseren()
{
	Pent=Jr=1970; Mn=6;
	RenteI=    600; RenteJ=RenteI*.0001; RenteM=RenteJ/12.;
	InlegM=    200;
	DeelnFactor = 1;
	Valuta=1000000;
	NMn=0; Wd0V=0;
	WdeI=WdeA=WdeV=Vermogen=0;
	Ex0T=0;
}

void	IntRenteBoeken()		/* Interne code 21 */
{
	long	rente;
	double  renteF;
	zdr		zDnr;
	
	zDnr=zDn0;
	while (zDnr)
	{
#ifdef CompXCode
		renteF = ((double)zDnr->Rcr)*RenteM+.001;
		if (renteF < 0) {
			renteF -= 1.0;
		}
		rente = renteF;
		/* Berent dit is de rekenmethode in het oude foef-programma! */
#else
		rente=floor(((double)zDnr->Rcr)*RenteM+.001); 
#endif /* CompXCode Dit was de methode die Harald gebruikte... */
		
		zDnr->Ren+=rente;
		zDnr->Rcr+=rente;
		zDnr=zDnr->vlg;
	}
}

void	BeginMaand()			/* Mutatie code 10 */
{
	if ((Debi*12+Cred)!=(Jr*12+Mn+1))
	 Fout(2,10,"datum volgt niet op vorige maand");
	Mn=Cred; Jr=Debi;
	IntRenteBoeken();
}

void	MutatieDng()			/* Mutatie code 31 */
{
	zdr		zDnr;

	if (Cred)
	{
		ZoekDnrRel(Cred, (void**)&zDnr);
		if (Aant > zDnr->Dng)
		{
			Fout(3,31,"Zoveel deelnemingen heeft deze persoon niet");
			return;
		}
		zDnr->Dng -= Aant;
		zDnr->Rcr += (Aant * WdeV);
	}
	
	if (Debi)
	{
		ZoekDnrRel(Debi, (void**)&zDnr);
		zDnr->Dng += Aant;
		zDnr->Rcr -= (Aant * WdeA);
		zDnr->Muk -= (Aant * (WdeA-WdeV));
	}
}

void	BerekenWaardeDng()	/* Interne code 30 */
{
	zdr		zDnr;
	zrl		zRel;

	DnT.Dng=0;
	
	zDnr=zDn0;
	while (zDnr) { 
		DnT.Dng+=zDnr->Dng; 
		zDnr=zDnr->vlg; 
	}
	
	Vermogen=0;
	
	zRel=zRe0;
	while (zRel)
	{
		if (zRel->Typ>=7) 
			zRel->Teg=0;
		if (zRel->Typ>=5) 
			zRel->Sld = zRel->Eff * zRel->Krs + zRel->Teg;
		
		Vermogen += zRel->Sld;
		
		if (zRel->Sld || zRel->Sto) zRel->Act = 1;
		
		zRel = zRel->vlg;
	}
	
	CreTot = DebTot = DnT.Bet = 0;
	
	zDnr = zDn0;
	while (zDnr)
	{
		if (zDnr->Rcr > 0) 
			CreTot += zDnr->Rcr;
		else
			DebTot -= zDnr->Rcr;
		Vermogen -= zDnr->Rcr;
		
		DnT.Bet += zDnr->Bet;
		
		zDnr = zDnr->vlg;
	}
	
	if (DnT.Dng) 
		WdeI = rondaf((double) Vermogen/DnT.Dng); 
	else 
		WdeI = 0;
	
	WdeA = ceil((double)WdeI*1.01);
	WdeV = floor((double)WdeI*0.99);
}

void	SplitsDng()				/* Mutatie code 03 */
{
	zdr		zDnr;
	
	if (Mucv < 90 && Mucv >= 10)
	 Fout(2,3,"Splitsing moet na code 90 en voor code 10");
	
	if (Bedrag == 0) Fout(4,3,"Splits: Bedrag mag niet 0 zijn");	
	if (Aant == 0) Fout(4,3,"Splits: Aantal mag niet 0 zijn");

	InlegM = Bedrag;
	DeelnFactor = DeelnFactor * Aant;
	
	DnT.Dng = 0;
	zDnr = zDn0;
	while (zDnr) {
		if (zDnr->Dng) {
			zDnr->Dng = zDnr->Dng * Aant;
			DnT.Dng += zDnr->Dng;
		}
		zDnr = zDnr->vlg;
	}
		
	InlD = rondaf(InlD/(float)Aant);
	ExwD = rondaf(ExwD/(float)Aant);
	Wd0V = rondaf(Wd0V/(float)Aant);
	if (DnT.Dng) 
		BerekenWaardeDng();
}

void	ValutaWyziging()		/* Mutatie code 02 */
{	
	zdr		zDnr;
	zrl		zRel;
	zpo	zP;

	if (Mucv<90 && Mucv>=10)
	 Fout(2,2,"Valutawijziging moet na code 90 en voor code 10");
	ValutF= Bedrag*mypow10(-Aant);
	Valuta=	Valuta*ValutF;
	Vermogen=rondaf(Vermogen*ValutF);
	InlegM=  rondaf(InlegM*ValutF);
	zDnr=zDn0;
	while (zDnr)
	{
		zDnr->Rcr=rondaf(zDnr->Rcr*ValutF);
		zDnr->Bet=rondaf(zDnr->Bet*ValutF);
		zDnr->Gdp=rondaf(zDnr->Gdp*ValutF);
		zDnr->Ove=rondaf(zDnr->Ove*ValutF);
		zDnr->Wid=rondaf(zDnr->Wid*ValutF);
		zDnr->Muk=rondaf(zDnr->Muk*ValutF);
		zDnr->Ren=rondaf(zDnr->Ren*ValutF);
		zDnr->Exw=rondaf(zDnr->Exw*ValutF);
		zDnr=zDnr->vlg;
	}
	zRel=zRe0;
	while (zRel)
	{
		zRel->Teg=rondaf(zRel->Teg*ValutF);
		zRel->Sto=rondaf(zRel->Sto*ValutF);
		zRel->Gsl=rondaf(zRel->Gsl*ValutF);
		zRel=zRel->vlg;
	}
	zP=zP0;
	while (zP)
	{
		zP->Som=rondaf(zP->Som*ValutF);
		zP=zP->vlg;
	}
	InlD=rondaf(InlD*ValutF);
	ExwD=rondaf(ExwD*ValutF);
	Ex0T=rondaf(Ex0T*ValutF);
	Wd0V=rondaf(Wd0V*ValutF);
	if (DnT.Dng) BerekenWaardeDng();
}


void	SetIntRente()			/* Mutatie code 20 */
{
	if (Mucv!=10) Fout(1,20,"Rente instellen bij voorkeur meteen na code 10");
	RenteI=Bedrag;

	RenteJ=RenteI*mypow10(-Aant);
	RenteI=rondaf(RenteJ*10000);
	if (RenteI<0 || RenteI>1200) Fout(1,20,"is deze rente wel goed ?");
	RenteM=RenteJ/12.;
}

void	SetPeriOver()			/* Mutatie code 25 */
{
	zdr		zDnr;
	zpo	zP;

	if (Debi>499 || Cred>499 || Debi<1 || Cred<1)
	{
		Fout(3,25,"nummer correspondeert niet met deelnemer");
		return;
	}
	ZoekPov(Debi, Cred, &zP);
	zP->Deb = Debi;
	zP->Cre = Cred;
	zP->Som=Bedrag;

	if (!ZoekDnrRel(Debi, (void**)&zDnr)) Fout(1,25,"naam Debiteur  staat niet in namenlijst");
	if (!ZoekDnrRel(Cred, (void**)&zDnr)) Fout(1,25,"naam Crediteur staat niet in namenlijst");
}

void	OverDng()				/* Mutatie code 32 */
{
	zdr		zDnr;

	if (Debi<1 || Debi>499 || Cred<1 || Cred>499)
	{
		Fout(3,Muco,"onmogelijke deelnemer");
		return;
	}
	if (!ZoekDnrRel(Debi, (void**)&zDnr)) Fout(1,25,"naam Debiteur  staat niet in namenlijst");
	zDnr->Dng += Aant;	
	zDnr->Bet += (Aant*WdeV);
	if (!ZoekDnrRel(Cred, (void**)&zDnr)) Fout(1,25,"naam Crediteur staat niet in namenlijst");
	zDnr->Dng -= Aant;	
	zDnr->Bet -= (Aant*WdeV);
}

void	Koers()           	/* Mutatie code 70 */
{
	zrl	zRel;
	
	if (Debi<700 || Debi>899)
	{
		Fout(3,Muco,"onmogelijk effectnummer");
		return;
	}
	ZoekDnrRel(Debi, (void**)&zRel);
	zRel->Krs = Bedrag;
}

void	EffBonus()           /* Mutatie code 73 */
{
	zrl	zRel;

	if (Debi<700 || Debi>899)
	{
		Fout(3,Muco,"onmogelijk effectnummer");
		return;
	}
	ZoekDnrRel(Debi, (void**)&zRel);
	zRel->Eff += Aant;
}

void	Transacties(int x)	/* Mutatie codes 22,23,27,51,52,61,62 */
									/*              ,64,71,81,82,83,84,86 */
{
	zdr		zDnr;
	zrl		zRel;

	if (x<500) /* het is een deelnemer */
	{
		if (!ZoekDnrRel(x, (void**)&zDnr)) {
			Fout(1,Muco,"naam deelnemer niet in namenlijst");
		}
		switch(Muco) {
			case kMutdnd:
			case kMuttfo:
			case kMutbfo: zDnr->Ove-=Bedrag;
				
			case kMutovb:
			case kMutove: zDnr->Rcr-=Bedrag; 
				          zDnr->Bet-=Bedrag;
						  break;
			default:	Fout(3,Muco,"onbekende of onjuiste mutatiecode");
		}
	} else if (x>899) {
		Fout(3,Muco,"onmogelijke relatie");
	} else {  /* het is een relatie */
		if (!ZoekDnrRel(x, (void**)&zRel)) {
			Fout(1,Muco,"naam relatie niet in namenlijst");
		}
		switch(Muco) {
			case kMutove:
			case kMutefh: 
				if (x>=700) { /* het is een effecten- of obligatiefonds */
					zRel->Eff+=Aant;
				}
			case kMutovb:
			case kMutbfo:
			case kMutbnb:
			case kMutbnk:
			case kMutdiv:
			case kMutdib:
			case kMutboc:
			case kMutpro:
			case kMutbwl: zRel->Sto+=Bedrag;
				
			case kMuttfo:
			case kMutbnr: zRel->Teg+=Bedrag;
						  break;

			default:	  Fout(3,Muco,"Onbekende of onjuiste mutatiecode");
		}
	}
}

void	PeriOverBoeken()		/* interne code 26 */
{
	zdr		zDnr;
	zpo		zP;

	zP=zP0;
	while (zP)
	{
		Bedrag=zP->Som;
		Cred=zP->Cre;
		Debi=zP->Deb;
		if (Debi!=0 && Cred!=0)
		{
			ZoekDnrRel(Debi, (void**)&zDnr);
			if (Bedrag) { zDnr->Rcr-=Bedrag; zDnr->Bet-=Bedrag; zDnr->Ove-=Bedrag; }
			ZoekDnrRel(Cred, (void**)&zDnr);
			if (Bedrag) { zDnr->Rcr+=Bedrag; zDnr->Bet+=Bedrag; zDnr->Ove+=Bedrag; }
		}
		zP=zP->vlg;
	}
}

void	MaandInleg()
{
	zdr		zDnr;

	zDnr=zDn0;
	while (zDnr)
	{
		zDnr->Rcr-=(InlegM*zDnr->Dng);
		zDnr=zDnr->vlg;
	}
	InlD+=InlegM;
}

void	Totaliseren()
{
	zdr		zDnr;
	zrl		zRel;

	BerekenWaardeDng();
	zDnr=zDn0;
	while (zDnr)
	{
		zDnr->Gdp+=rondaf((zDnr->Dng*WdeV+zDnr->Rcr-zDnr->Gdp)/NMn);
		if (zDnr->Dng || zDnr->Rcr) zDnr->Act=1;
		zDnr=zDnr->vlg;
	}
	DnT.Exw=0;
	FuT.Sld=FuT.Gsl=FuT.Sto=0;
	BaT.Sld=BaT.Gsl=BaT.Sto=0;
	EfT.Sld=EfT.Gsl=EfT.Sto=0;
	ObT.Sld=ObT.Gsl=ObT.Sto=0;
	ReT.Sld=ReT.Gsl=ReT.Sto=0;
	zRel=zRe0;
	while (zRel)
	{
		zRel->Sld = zRel->Teg + zRel->Eff*zRel->Krs;
		zRel->Gsl+=rondaf((zRel->Sld - zRel->Gsl)/NMn);
		if (zRel->Sld || zRel->Sto) zRel->Act=1;
		if (zRel->Typ==5)
		{
			FuT.Sld+=zRel->Sld;
			FuT.Gsl+=zRel->Gsl;
			FuT.Sto+=zRel->Sto;
		}
		else DnT.Exw+=(zRel->Sld-zRel->Sto);
		if (zRel->Typ==6)
		{
			BaT.Sld+=zRel->Sld;
			BaT.Gsl+=zRel->Gsl;
			BaT.Sto+=zRel->Sto;
		}
		if (zRel->Typ==7)
		{
			EfT.Sld+=zRel->Sld;
			EfT.Gsl+=zRel->Gsl;
			EfT.Sto+=zRel->Sto;
		}
		if (zRel->Typ==8)
		{
			ObT.Sld+=zRel->Sld;
			ObT.Gsl+=zRel->Gsl;
			ObT.Sto+=zRel->Sto;
		}
		ReT.Sld+=zRel->Sld;
		ReT.Gsl+=zRel->Gsl;
		ReT.Sto+=zRel->Sto;
		zRel=zRel->vlg;
	}
	if (DnT.Dng)
	{
		ExwV=(DnT.Exw-Ex0T)/DnT.Dng;
		DnT.Exw= Ex0T+ExwV*DnT.Dng;
	}
	DnT.Gdp=0; DnT.Dep=0;
	zDnr=zDn0;
	while (zDnr)
	{
		zDnr->Exw+=(ExwV*zDnr->Dng);
		DnT.Gdp+=zDnr->Gdp;
		DnT.Dep+=(zDnr->Dng*WdeV+zDnr->Rcr);
		zDnr->Wid+=(WdeV-Wd0V-InlegM)*zDnr->Dng;
		zDnr=zDnr->vlg;
	}
	ExwD+=ExwV;
}

void	WriteBasisFile()
{
	char	 Basnaam[120];
	FILE	*BasisF;
	zdr		zDnr;
	zrl		zRel;
	zpo		zP;
	int		t;
		
	sprintf(Basnaam,"%s%4d%c.dta","basis/bas",Jr,'a'-1+Mn); /* Berent: '@' is een andere code op de mac */
	if (!(BasisF=fopen(Basnaam,"w"))) {
		Fout(4,0,"Kan Basis bestand niet openen");
	}

	fprintf(BasisF
	,"jaar mn inlm dnspl valuta verkwaarde  inleg/dln   vermogen  exwin/dln  winst/dln debiteuren\n");
	fprintf(BasisF,"%4d %2d %4d %4d %7ld %10ld %10ld %10ld %10ld %10ld %10ld\n"
	,Jr,Mn,InlegM,DeelnFactor,Valuta,WdeV,InlD,Vermogen,ExwD,WdeV-InlD,DebTot);

	fprintf(BasisF
	,"rente   aantal crediteuren  deposito    betaald  gem depos  ext winst\n");
	fprintf(BasisF,"%4d    %6ld %10ld %10ld %10ld %10ld %10ld\n",
	RenteI,DnT.Dng,CreTot,DnT.Dep,DnT.Bet,DnT.Gdp,DnT.Exw);

	fprintf(BasisF,"deelnr  aantal    rek-crt   deposito    betaald  gem depos");
	fprintf(BasisF,"  ext winst  winst dln  mutkost rc rente   overboek\n");
	zDnr=zDn0;
	while (zDnr)
	{
		if (zDnr->Act)
		 fprintf(BasisF
		 ,"%03d %-3s %6ld %10ld %10ld %10ld %10ld %10ld %10ld %8ld %8ld %10ld\n"
		 ,zDnr->Idx,zDnr->Dnm,zDnr->Dng,zDnr->Rcr,zDnr->Dng*WdeV+zDnr->Rcr
		 ,zDnr->Bet,zDnr->Gdp,zDnr->Exw,zDnr->Wid,zDnr->Muk,zDnr->Ren,zDnr->Ove);
		zDnr=zDnr->vlg;
	}

	fprintf(BasisF,"relatie aantal     tegoed      saldo    gestort  gem saldo\n");
	zRel=zRe0;
	while (zRel)
	{
		if (zRel->Act)
		 fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
		 ,zRel->Idx,zRel->Rnm,zRel->Eff,zRel->Teg,zRel->Sld
		 ,zRel->Sto,zRel->Gsl);
		zRel=zRel->vlg;
	}
	fprintf(BasisF,"groepen\n");
	fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
	,FuT.Idx,FuT.Rnm,FuT.Eff,FuT.Teg,FuT.Sld,FuT.Sto,FuT.Gsl);
	fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
	,BaT.Idx,BaT.Rnm,BaT.Eff,BaT.Teg,BaT.Sld,BaT.Sto,BaT.Gsl);
	fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
	,EfT.Idx,EfT.Rnm,EfT.Eff,EfT.Teg,EfT.Sld,EfT.Sto,EfT.Gsl);
	fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
	,ObT.Idx,ObT.Rnm,ObT.Eff,ObT.Teg,ObT.Sld,ObT.Sto,ObT.Gsl);
	fprintf(BasisF,"%3d %.3s %6ld %10ld %10ld %10ld %10ld\n"
	,ReT.Idx,ReT.Rnm,ReT.Eff,ReT.Teg,ReT.Sld,ReT.Sto,ReT.Gsl);

	fprintf(BasisF,"pov deb    cre     bedrag\n");
	zP=zP0; t=0;
	while (zP)
	{
		if (zP->Som)
		 fprintf(BasisF,"%3d %3d %6d %10ld\n",++t,zP->Deb,zP->Cre,zP->Som);
		zP=zP->vlg;
	}
	
	/*
	 Frank:
	 
	 Hier de overschrijvingen wegschrijven.
	 Waarschijnlijk het handigst om een lijst weg te schrijven met debiteur en crediteur plus bedrag en aantal, 
	 ook voor deelneming aan- en verkoop.
	 Deze lijst moet elke maand worden opgebouwd bij het verwerken van de transacties in deze FBereken.c
	 Zie hiervoor de voorbeelden in de oude 'FOEF Reken.c' code.
	 
	 Die lijst vervolgens voor alle maanden in een periode in FUitvoer.c inlezen en opslaan in een 
	 dubbele array long overboekPeriode[maxDlnrs][MaxDlnrs];
	 
	 En dan wegschrijven: (onderstaande code komt dan in FUitvoer.c)
	 
	 char		koop[2][9]   = {"gekocht ","verkocht"};
	 char		actie[2][12] = {"gestort:   ","uitgekeerd:"};
	 char		vanAan[2][4] = {"van","aan"};

	 sprintf(pr,"\r\t2. Aan/verkoop deelnemingen\r");Display(ctopstr(pr, pr2));
	 if ((deelNr->lastPAantDeeln > 0) || (deelNr->lastPAantDeeln < 0)) {
	 sprintf(pr,"\t\t%s %2d deeln.:%29.2f\r",
	 koop[deelNr->lastPAantDeeln >= 0 ? 0 : 1], 
	 deelNr->lastPAantDeeln >= 0 ? deelNr->lastPAantDeeln : 
	 -deelNr->lastPAantDeeln,
	 deelNr->verkBedrDlngPeriode/(float)100.0);Display(ctopstr(pr, pr2));
	 }
	 else {
	 sprintf(pr, "\t\tGeen.\r");Display(ctopstr(pr, pr2));
	 }
	 sprintf(pr,"\r\t3. Stortingen/uitkeringen/overschrijvingen\r");Display(ctopstr(pr, pr2));
	 if (((somAfEnBijGestort = deelNr->gestortPeriode) != 0)) {
	 sprintf(pr,"\t\t%s%37.2f\r", 
	 actie[somAfEnBijGestort > 0.0 ? 0 : 1], somAfEnBijGestort/(float)100.0);Display(ctopstr(pr, pr2));
	 }
	 for (j = 0; j < MaxDlnrs; j++) {
	 if (overboekPeriode[deelNr->deelnrNr*MaxDlnrs+j]) {
	 deelNr2 = FindDeelnemer(j);
	 somAfEnBijGestort += overboekPeriode[deelNr->deelnrNr*MaxDlnrs+j];
	 sprintf(pr,"\t\tOverschrijving %s %s: %25.2f\r",
	 vanAan[overboekPeriode[deelNr->deelnrNr*MaxDlnrs+j] >= 0 ? 0 : 1], 
	 ptocstr(deelNr2->afk,text), overboekPeriode[deelNr->deelnrNr*MaxDlnrs+j]/(float)100.0);Display(ctopstr(pr, pr2));
	 }
	 }
	 */
	
	fclose(BasisF);
}

void	EindeMaand() 			/* Mutatie code 90 */
{
	if (Debi!=Jr || Cred!=Mn)
	 Fout(2,10,"datum is ongelijk aan begin van de maand");
	NMn++;
	PeriOverBoeken();
	MaandInleg();
	Totaliseren();
	WriteBasisFile();
	Ex0T=DnT.Exw; Wd0V=WdeV;
}

int	Controle()
{
	int	d,c,dc;
	if (Muco<0 || Debi<0 || Cred<0 || Bedrag<0 || Aant<0)
	 return Fout(3,Muco,"negatieve waarde");
	c=Cred;
	if (c<1) c=0;			// geen crediteur
	else if (c<500) c=1;	// deelnemer
	else if (c<600) c=2;	// fonds
	else if (c<700) c=3;	// bankrekening
	else if (c<900) c=4;	// effecten- of obligaties
	else c=5;				// komt niet voor
	d=Debi;
	if (d<1) d=0;			// geen debiteur
	else if (d<500) d=1;	// deelnemer
	else if (d<600) d=2;	// fonds
	else if (d<700) d=3;	// bankrekening
	else if (d<900) d=4;	// effecten- of obligaties
	else d=5;				// jaartal
	dc=d*10+c;
	
	if ((d==5 && Muco!=kMutbeg && Muco!=kMutein) || c==5)
	 return Fout(3,Muco,"onmogelijke instelling");
	
	if (dc==0 && Muco!=kMutbeg && Muco!=kMutein && Muco!=kMutval && Muco!=kMutspl && Muco!=kMutint)
	 return Fout(2,Muco,"debiteur en crediteur 0");
	
	if ((dc==10 || dc== 1) && Muco!=kMutmdn)
	 return Fout(3,Muco,"moet een code 31 (kMutmdn) zijn");
	
	if ((dc==20 || dc== 2) && Muco!=kMuttfo)
	 return Fout(3,Muco,"moet code 51 (kMuttfo) zijn");
	
	if ((dc==30 || dc== 3) && Muco!=kMutbnr && Muco!=kMutbnk)
	 return Fout(3,Muco,"moet code 61 (kMutbnr) of 64 (kMutbnk) zijn");
	
	if ((dc==40 || dc== 4) && Muco!=kMutkef && Muco!=kMutboe)
	 return Fout(3,Muco,"moet code 70 (kMutkef) of 73 (kMutboe) zijn");
	
	if ( dc==11 && Muco!=kMutdnd && Muco!=kMutipo && Muco!=kMutovd)
	 return Fout(3,Muco,"moet code 23 (kMutdnd), 25 (kMutipo) of 32 (kMutovd) zijn");
	
	if ((dc==21 || dc== 12) && Muco!=kMuttfo && Muco!=kMutbfo)
	 return Fout(3,Muco,"moet code 51 (kMuttfo) of 52 (kMutbfo) zijn");
	
	if ((dc==31 || dc== 13) && Muco!=kMutovb)
	 return Fout(3,Muco,"moet code 22 (kMutovb) zijn");
	
	if ((dc==41 || dc==14) && Muco!=kMutove)
	 return Fout(3,Muco,"moet code 27 (kMutove) zijn");
	
	if ((dc==32 || dc==23) && Muco!=kMutbfo)
	 return Fout(3,Muco,"moet code 52 (kMutbfo) zijn");
	
	if ( dc==33 && Muco!=kMutbnb)
	 return Fout(3,Muco,"moet code 62 (kMutbnb) zijn");
	
	if ((dc==43 || dc==34) && (Muco<kMutefh || Muco==kMutboe))
	 return Fout(3,Muco,"moet code 71 (kMutefh) of boven 73 (kMutboe) zijn");
	
	return 0;
}

void	GeldVerkeer()
{
	if (Debi != 0) Transacties(Debi);
	Bedrag = -Bedrag;	
	Aant =- Aant;
	if (Cred != 0) Transacties(Cred);
}

void	Decoderen()
{
	char *lMrec = Mrec;
	zdr	zDnrRel;
	
	if (*lMrec == '\r') lMrec++; /* Dit omdat op de Mac er soms een '\r' vooraan Mrec staat */
	
	Muco=atoi(lMrec   );
	if (lMrec[6] >= '0' && lMrec[6] <= '9') {
		Debi = atoi(lMrec + 2);
	} else {
		ZoekDnrRelByName(lMrec + 2, (void**)&zDnrRel);
		Debi = zDnrRel ? zDnrRel->Idx : 0;
	}
	if (lMrec[11] >= '0' && lMrec[11] <= '9') {
		Cred = atoi(lMrec + 7);
	} else {
		ZoekDnrRelByName(lMrec + 7, (void**)&zDnrRel);
		Cred = zDnrRel ? zDnrRel->Idx : 0;
	}
	
	
	Bedrag=atol(lMrec+12);
	Aant=atoi(lMrec+23);
	if (Controle()>2) 
		return;
	switch(Muco)
	{
		case kMutval:	ValutaWyziging();	break;
		case kMutspl:	SplitsDng(); 		break;
		case kMutbeg:	BeginMaand(); 		break;
		case kMutint:	SetIntRente();		break;
		case kMutipo:	SetPeriOver();		break;
		case kMutmdn:	MutatieDng(); 		break;
		case kMutovd:	OverDng();			break;
		case kMutkef:	Koers();			break;
		case kMutboe:	EffBonus();			break;
		case kMutein:	EindeMaand(); 		break;
		default:	GeldVerkeer();
	}
	Mucv=Muco;
}
/* kopie uit FBasis.h:
 #define kMutval	2	// Valutawijziging
 #define kMutspl	3	// Basisinleg p/mnd
 #define kMutbeg	10	// Begin maand
 #define kMutint	20	// Nieuwe int rente
 #define kMutbin	21	// Boeken int rente
 #define kMutovb	22	// Overb dlnr-bank
 #define kMutdnd	23	// Overb dlnr-dlnr
 #define kMutipo	25	// Nieuwe per-overb
 #define kMutove	27	// Overdr effecten
 #define kMutmdn	31	// Mutatie deeln
 #define kMutovd	32	// Overboeken deeln
 #define kMuttfo	51	// Toewijzing fonds
 #define kMutbfo	52	// Betaling fonds
 #define kMutbnr	61	// Bankrente
 #define kMutbnb	62	// Overb bank-bank
 #define kMutbnk	64	// Bankkosten
 #define kMutkef	70	// Koers effecten
 #define kMutefh	71	// Effectenhandel
 #define kMutboe	73	// Effectenbonus
 #define kMutdiv	81	// Dividend
 #define kMutdib	82	// Dividendbel
 #define kMutboc	83	// Bonus contant
 #define kMutpro	84	// Provisie
 #define kMutbwl	86	// Bewaarloon
 #define kMutein	90	// Einde maand
 */


#define LetterMutaties

			/* Met deze functie wordt een copie van de laatste muta-file		*/
			/* gemaakt, aangevuld met een extra maand in concept, voorzien		*/
			/* van de te verwachten codes 10, 22, 31 en 90							*/
void	MaakMuta()
{
	char	 HulpChar[3000];
	int	 Aankoop;
	int	 t=0,nt,LaatsteMaand=0;
	FILE	*MnewF, *MutaF;
	char	 Mutanaam[122];
	zdr		zDnr;
	char *lMrec;
	zdr	zDnrRel;
	
	Pent-=5;
	t=0;
	sprintf(Mutanaam,"%s%4d.dta","muta/muta",Pent);
	MutaF=fopen(Mutanaam,"r");
	if (MutaF==NULL) return;
	sprintf(Mutanaam,"%s%4d.cor","muta/muta",Pent);
	MnewF=fopen(Mutanaam,"w");
	while (fgets(Mrec,60,MutaF)!=NULL)
	{
		if (*lMrec == '\n') {
			fprintf(MnewF,"\n"); // Kopieer lege regels gewoon
			continue;
		}
		lMrec = Mrec;
		if (*lMrec == '\r') lMrec++; /* Dit omdat op de Mac er soms een '\r' vooraan Mrec staat */

		Muco=atoi(lMrec   ); 
		
		if (lMrec[6] >= '0' && lMrec[6] <= '9') {
			Debi = atoi(lMrec + 2);
		} else {
			ZoekDnrRelByName(lMrec + 2, (void**)&zDnrRel);
			Debi = zDnrRel ? zDnrRel->Idx : 0;
		}
		if (lMrec[11] >= '0' && lMrec[11] <= '9') {
			Cred = atoi(lMrec + 7);
		} else {
			ZoekDnrRelByName(lMrec + 7, (void**)&zDnrRel);
			Cred = zDnrRel ? zDnrRel->Idx : 0;
		}
		Bedrag=atol(lMrec+12);
		Aant=atoi(lMrec+23);
		
		fprintf(MnewF,"%.28s\n",lMrec);
		if (Muco==kMutbeg && Debi == Jr && Cred == Mn) {
			LaatsteMaand = 1;
			Mn++;
			if (Mn>12) { 
				Jr++; Mn=1;
			}
		}
		if (!LaatsteMaand) continue;
		
		if (Muco==kMutbeg || Muco==kMutein) {
			sprintf(HulpChar+30*t++,"%2d %4d %4d          0    0 *",Muco,Jr,Mn);
			if (Muco==kMutbeg)
			{
				zDnr=zDn0;
				while (zDnr && t < 100)
				{
					Aankoop = 0;
					while (zDnr->Rcr - Aankoop * WdeA > WdeA) Aankoop++;
					if (Aankoop)
#ifdef LetterMutaties
						sprintf(HulpChar+30*t++,"31 %4s    0          0 %4d *", zDnr->Dnm, Aankoop);
#else LetterMutaties
					sprintf(HulpChar+30*t++,"31 %4d    0          0 %4d *", zDnr->Idx, Aankoop);
#endif LetterMutaties
					zDnr = zDnr->vlg;
				}
			}
		} else if (Muco == kMutovb || Muco == kMutkef || Muco == kMutbnk) {
#ifdef LetterMutaties
			zdr zDebi, zCred;
			
			ZoekDnrRel(Debi, (void**)&zDebi);
			ZoekDnrRel(Cred, (void**)&zCred);
			sprintf(HulpChar+30*t++,"%2d %4s %4s %10ld %4d *", 
					Muco, 
					zDebi ? zDebi->Dnm : "  0", 
					zCred ? zCred->Dnm : "  0",
					Bedrag,
					Aant);
#else LetterMutaties
			//sprintf(HulpChar+30*t++,"%.28s *",lMrec);
			sprintf(HulpChar+30*t++,"%2d %4d %4d %10ld %4d *", 
					Muco, 
					Debi, 
					Cred,
					Bedrag,
					Aant);
#endif LetterMutaties
		}
	}
	nt=t; t=0;
	while (t<nt) fprintf(MnewF,"%.30s\n",HulpChar+30*t++);
	fclose(MutaF); fclose(MnewF);
}

void	MaakAllMuta()
{
	int		lLastPent;
	FILE	*MnewF, *MutaF;
	char	Mutanaam[122];
	char	*lMrec;
	zdr		zDnrRel;
	zdr		zDebi, zCred;
	
	lLastPent = Pent - 5;
	Pent = 1970;
	
	while (Pent <= lLastPent) {
		sprintf(Mutanaam,"%s%4d.dta","muta/muta",Pent);
		MutaF=fopen(Mutanaam,"r");
		if (MutaF == NULL) 
			return;
		sprintf(Mutanaam,"%s%4d.cor","muta/muta",Pent);
		MnewF=fopen(Mutanaam,"w");
		
		while (fgets(Mrec,60,MutaF)!=NULL)
		{
			lMrec = Mrec;
			if (*lMrec == '\r') lMrec++; /* Dit omdat op de Mac er soms een '\r' vooraan Mrec staat */
			
			if (strlen(lMrec) < 25) { // lege regels mogen!
				fprintf(MnewF,"%.28s\n",lMrec);
			} else {
				Muco = atoi(lMrec   ); // de mutatiecode
				
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

	#ifdef LetterMutaties
				if (Muco == kMutbeg || Muco == kMutein) {
					fprintf(MnewF,"%2d %4d %4d %10ld %4d\n", 
							Muco, 
							Debi, 
							Cred,
							Bedrag,
							Aant);
				} else {
					ZoekDnrRel(Debi, (void**)&zDebi);
					ZoekDnrRel(Cred, (void**)&zCred);
					fprintf(MnewF,"%2d %4s %4s %10ld %4d\n", 
						Muco, 
						zDebi ? zDebi->Dnm : "  0", 
						zCred ? zCred->Dnm : "  0",
						Bedrag,
						Aant);
				}
	#else LetterMutaties
				//sprintf(HulpChar+30*t++,"%.28s *",lMrec);
				fprintf(MnewF,"%2d %4d %4d %10ld %4d\n", 
						Muco, 
						Debi, 
						Cred,
						Bedrag,
						Aant);
	#endif LetterMutaties
				if (Muco == kMutein) {
					fprintf(MnewF,"\n"); // lege regel na elke maand
				}
					
			}
			
		}			
		fclose(MnewF);
		Pent += 5;
	}
	fclose(MutaF); 
}

int main()
{
	Errs=0;
	LeesNamen();
	char	 Mutanaam[122];
	FILE	*MutaF;

#ifndef CompXCode
	clrscr();
#endif
	
	Initialiseren();
	while (1)
	{
		sprintf(Mutanaam,"%s%4d.dta","muta/muta",Pent);
		MutaF=fopen(Mutanaam,"r");
		if (MutaF==NULL) break;
		
		while (fgets(Mrec,60,MutaF)!=NULL) {
			if (strlen(Mrec) > 25) Decoderen(); /* berent: lege regels mogen! */
		}
		fclose(MutaF);
		Pent+=5;
	}

//#define ConverteerMuta /* als deze aanstaat wordt van alle mutafiles een nieuwe versie aangemaakt */
	
#ifdef ConverteerMuta
	MaakAllMuta();
#else ConverteerMuta
	if (Muco==kMutein) MaakMuta();
#endif ConverteerMuta
	
	return 0;
}
