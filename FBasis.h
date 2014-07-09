/*
 *  FBasis.h
 *  FUitvoer
 *
 *  Created by Berent Daan on 14-09-10.
 *  Copyright 2010 Prive. All rights reserved.
 *
 */


#ifndef _H_FBasis
#define _H_FBasis

#define CompXCode /* aan zetten voor compilatio onder XCode for MaXC OS X */

/* defs voor mutaties */

#define kMutval	2	// Valutawijziging
#define kMutspl 3	// Basisinleg p/mnd
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

/* LET OP: de eerste velden van deze structs zijn allemaal het zelfde.
   Dit om ze op de zelfde wijze te kunnen behandelen.
   Dit had eigenlijk een union moeten zijn, moet nog worden aangepast
 */

typedef struct dnr
{
	struct dnr	*vlg,*vor;		/* pointer naar volgende en vorige deelnemer		*/
	int		Idx, Act;			/* index nummer deelnemer								*/
	char	Dnm[4], Naam[21],	/* afkorting en volledige naam deelnemer			*/
			Email[31];
			/* Financiële informatie. De velden met de 1-extentie worden gebruikt om periode
			   overzichten te maken: begin- en eindwaarden worden beide in hetzelfde record ingelezen.
			   dit moet nog worden aangepast! */
	long	Dng,Dng1,			/* aantal deelnemingen									*/
			Rcr,Rcr1,			/* rekening-courant										*/
			Dep,Dep1,			/* totaal deposito Dep=Rcr+Dng*WdeV					*/
			Bet,Bet1,			/* in totaal aan FOEF betaald							*/
			Gdp,Gdp1,			/* gemiddeld deposito sinds juli 1970				*/
			Exw,Exw1,			/* totale externe winst									*/
			Wid,Wid1,			/* totale winst op deelnemingenbezit				*/
			Muk,Muk1,			/* aankoopkosten van deelnemingen (1%)				*/
			Ren,Ren1,			/* totale interne rente									*/
			Ove,Ove1;			/* totaal overschrijvingen								*/
} dnr,*zdr;

typedef struct rel
{
	struct rel	*vlg,*vor;		/* pointer naar volgende en vorige relatie		*/
	int		Idx,Act;			/* index relatie 		*/
	char	Rnm[4], Naam[21];	/* 3-letter aanduiding relatie en naam				*/
	int		Typ;				/* Type relatie (5=fonds, 6=bank enz) 
								   Is in feite index/100. 
								   Moeten nog nette defs voor gemaakt worden */
	long	Eff,Eff1,			/* aantal aandelen op dit effect						*/
			Teg,Teg1,			/* tegoed bij deze relatie								*/
			Sld,Sld1,			/* totaal saldo Sld=Teg+Eff*Koers					*/
			Sto,Sto1,			/* totaal betaald tgv deze relatie					*/
			Gsl,Gsl1,			/* gemiddeld saldo bij deze relatie					*/
			Krs;
} rel,*zrl;

/* gegevens mutaties									*/
typedef struct mut
{
	struct mut	*vlg,*vor;		/* pointer naar volgende en vorige mutatie		*/
	int		Idx,Act;			/* indexcode mutatie + 900 	*/
	char	Rnm[4], Naam[21];	/* 3-letter aanduiding relatie						*/
} mut,*zmut;

/* gegevens periodieke overschrijvingen									*/
typedef struct pov
{
	struct	pov *vlg,*vor;
	int		Deb,Cre;
	long	Som;
} pov,*zpo;

/* pointers worden aangegeven met een 'z' als eerste letter				*/
extern dnr	 mDnr;				/* model deelnemer geinitialiseerd		*/
extern dnr	 DnT;				/* totaal alle Dn		*/
extern zdr	 zDn0;				/* pointer naar eerste deelnemer		*/

extern rel	 mRel;				/* model relatie geinitialiseerd		*/
extern zrl	 zRe0;				/* pointer naar  eerste relatie			*/
extern rel	 ReT,FuT,BaT,EfT,ObT;	/* relatie alg, fondsen, banken, effecten, obl	*/

extern pov	mPov;				/* model periodieke overschrijving geinitialiseerd		*/
extern zpo	zP0;				/* pointer naar  eerste periodieke overschrijving			*/

/* Zoekt de pointer van een deelnemer en maakt zonodig een nieuwe		*/
extern int	AddDnr(int, zdr*);

/* Zoekt de pointer van een relatie en maakt zonodig een nieuwe			*/
extern int	AddRel(int, zrl*);

/* Zoekt de pointer van een deelnemer of relatie (maakt geen nieuwe) returns 1 if found, 0 if not	*/
extern int	ZoekDnrRel(int, void **);
/* Zoekt de pointer van een deelnemer of relatie op naam (maakt geen nieuwe)	*/
extern int	ZoekDnrRelByName(char *, void **);

/* Zoekt de pointer van een mutatie (maakt geen nieuwe) returns 1 if found, 0 if not	*/
extern int	ZoekMuta(int, zmut *);

/* Zoekt de pointer van een pov en maakt zonodig een nieuwe			*/
extern int	ZoekPov(int, int, zpo *);

extern long rondaf(double);

#ifdef CompXCode

extern double mypow10(int aPow);

#endif // CompXCode

extern int GetFieldString(char *, char *);

/* inlezen namen file en geheugen-allocatie								*/
extern int	LeesNamen();

#endif _H_FBasis

