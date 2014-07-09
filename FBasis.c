/*
 *  FBasis.c
 *  FUitvoer
 *
 *  Created by Berent Daan on 14-09-10.
 *  Copyright 2010 Prive. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "FBasis.h"


/* Globale varialbelen				*/

dnr	 mDnr=					/* model deelnemer geinitialiseerd					*/
{NULL,NULL,0,0,"???","xxx                 ", ""
	,0,0,0,0,0,0,0,0,0,0
	,0,0,0,0,0,0,0,0,0,0};
zdr	 zDn0;					/* pointer naar eerste deelnemer */
dnr	 DnT;					/* Totaal alle deelnemers					*/

rel	mRel=					/* model relatie geinitialiseerd						*/
{NULL,NULL,0,0,"???",0,0,0,0,0,0,0,0,0,0,0,0};
zrl	 zRe0;					/* pointer naar eerste relatie		*/
rel	 ReT,FuT,BaT,EfT,ObT;	/* Totaal relaties, fondsen, banken, effecten, obl	*/

mut	mMuta=					/* model mutatie geinitialiseerd						*/
{NULL,NULL,0,0,"???","xxx                 "};
zmut	zMu0;				/* pointer naar eerste mutatie		*/


pov	 mPov={NULL,NULL,0,0,0};
zpo	 zP0;					/* pointer naar eerste periodieke overschrijving		*/


/* Zoekt de pointer van een deelnemer en maakt zonodig een nieuwe	*/
int	AddDnr(int iDnr, zdr *aDnrHndl)
{
	int lNew = 0;
	zdr	lDnrPtr, pvor;
	
	lDnrPtr = pvor = zDn0;
	
	while (lDnrPtr && lDnrPtr->Idx!=iDnr) { pvor=lDnrPtr; lDnrPtr=lDnrPtr->vlg; }
	if (lDnrPtr == NULL)
	{
		lDnrPtr=(dnr*)malloc(sizeof(dnr));
		if (lDnrPtr==NULL)
		{
			printf("Geen geheugenruimte voor deelnemers\n");
			exit(1);
		}
		*lDnrPtr=mDnr;
		if (zDn0 == NULL) 
			zDn0 = lDnrPtr;
		lDnrPtr->vlg = NULL; lDnrPtr->vor=pvor;
		if (pvor) pvor->vlg = lDnrPtr;
		lDnrPtr->Idx = iDnr;
		lNew = 1;
	}
	*aDnrHndl = lDnrPtr;
	return lNew;
}

/* Zoekt de pointer van een relatie en maakt zonodig een nieuwe	*/
int	AddRel(int iRel, zrl *aRelHndl)
{
	int lNew = 0;
	zrl	lRelPtr, pvor;
	
	lRelPtr = pvor = zRe0;
	while (lRelPtr && lRelPtr->Idx!=iRel) { 
		pvor = lRelPtr; 
		lRelPtr = lRelPtr->vlg; 
	}
	if (lRelPtr == NULL)
	{
		lRelPtr = (rel*)malloc(sizeof(rel));
		if (lRelPtr == NULL)
		{
			printf("Geen geheugenruimte voor relaties\n");
			exit(1);
		}
		*lRelPtr = mRel;
		if (zRe0 == NULL) 
			zRe0 = lRelPtr;
		lRelPtr->vlg = NULL; lRelPtr->vor = pvor;
		if (pvor) pvor->vlg = lRelPtr;
		lRelPtr->Idx = iRel;
		lNew = 1;
	}
	*aRelHndl = lRelPtr;
	
	return lNew;
}


/* Zoekt de pointer van een Mutatie en maakt zonodig een nieuwe	*/
int	ZoekMutaNew(int iMuta, zmut *aMutaHndl)
{
	int lNew = 0;
	zmut	lMutaPtr, pvor;
	
	lMutaPtr = pvor = zMu0;
	while (lMutaPtr && lMutaPtr->Idx != iMuta) { 
		pvor = lMutaPtr; 
		lMutaPtr = lMutaPtr->vlg; 
	}
	if (lMutaPtr == NULL) {
		lMutaPtr = (mut*)malloc(sizeof(mut));
		if (lMutaPtr == NULL)
		{
			printf("Geen geheugenruimte voor Mutaties\n");
			exit(1);
		}
		*lMutaPtr = mMuta;
		if (zMu0 == NULL) 
			zMu0 = lMutaPtr;
		lMutaPtr->vlg = NULL; lMutaPtr->vor = pvor;
		if (pvor) pvor->vlg = lMutaPtr;
		lMutaPtr->Idx = iMuta;
		lNew = 1;
	}
	*aMutaHndl = lMutaPtr;
	
	return lNew;
}

/* Zoekt de pointer van een Mutatie (maakt geen nieuwe) */
int	ZoekMuta(int iMuta, zmut *aMutaHndl)
{
	zmut	lMutaPtr;
	
	lMutaPtr = zMu0;
	while (lMutaPtr && lMutaPtr->Idx != iMuta) { 
		lMutaPtr = lMutaPtr->vlg; 
	}
	*aMutaHndl = lMutaPtr;
	
	return (*aMutaHndl != NULL);
}

/* Zoekt de pointer van een deelnemer of relatie (maakt geen nieuwe)	*/
int	ZoekDnrRel(int aRel, void **aDnrHndl)
{
	zdr	lDnrPtr;
	
	lDnrPtr = zDn0;
	while (lDnrPtr && lDnrPtr->Idx!=aRel) { 
		lDnrPtr=lDnrPtr->vlg; 
	}
	if (lDnrPtr == NULL) {
		lDnrPtr = (zdr)zRe0;
		while (lDnrPtr && lDnrPtr->Idx != aRel) { 
			lDnrPtr = lDnrPtr->vlg; 
		}
	}
	if (lDnrPtr == NULL) {
		lDnrPtr = (zdr)zMu0;
		while (lDnrPtr && lDnrPtr->Idx != aRel) { 
			lDnrPtr = lDnrPtr->vlg; 
		}
	}
	*aDnrHndl = (void *)lDnrPtr;
	
	return (*aDnrHndl != NULL);
}


/* Zoekt de pointer van een deelnemer of relatie op naam (maakt geen nieuwe)	*/
int	ZoekDnrRelByName(char *aRel, void **aDnrHndl)
{
	zdr	lDnrPtr;
	char *lChar = aRel, *lSChar, lSearchNm[4];
	
	while (*lChar == ' ') lChar++; //skip preceding spaces
	lSChar = lSearchNm;
	while (*lChar != ' ') {
		*lSChar++ = *lChar++;
	}
	*lSChar = 0;
		
	lDnrPtr = zDn0;
	while (lDnrPtr && strcmp(lDnrPtr->Dnm, lSearchNm)) { 
		lDnrPtr=lDnrPtr->vlg; 
	}
	if (lDnrPtr == NULL) {
		lDnrPtr = (zdr)zRe0;
		while (lDnrPtr && strcmp(lDnrPtr->Dnm, lSearchNm)) { 
			lDnrPtr = lDnrPtr->vlg; 
		}
	}
	*aDnrHndl = (void *)lDnrPtr;
	
	return (*aDnrHndl != NULL);
}


/* Zoekt de pointer van een per.overschr. of maakt een nieuwe		*/
int	ZoekPov(int iCre,int iDeb, zpo *aPoPtr)
{
	int lNew = 0;
	zpo	zP;
	zpo	pvor;
	
	*aPoPtr = NULL;
	
	pvor=zP=zP0;
	
	while (zP && 
		   !((zP->Cre == iCre && zP->Deb == iDeb) || 
			 (zP->Cre == iDeb && zP->Deb == iCre)) ) {
		pvor=zP;
		zP=zP->vlg;
	}
	
	if (!zP) {
		pvor=zP=zP0;
		while (zP && 
			   !(zP->Cre == 0 || zP->Deb == 0 || zP->Som == 0)) {
			pvor=zP; 
			zP=zP->vlg;
		}
	}
	
	if (!zP) {
		zP=(pov*)malloc(sizeof(pov));
		if (zP==NULL) {
			printf("Geen geheugenruimte voor periodieke overschrijvingen\n");
			exit(1);
		}
		*zP=mPov;
		if (zP0==NULL) 
			zP0=zP;
		zP->vlg = NULL;
		zP->vor=pvor;
		if (pvor) pvor->vlg=zP;
		
		lNew = 1;
	}
	
	
	*aPoPtr = zP;
	
	return lNew;
}


long	rondaf(double fl)
{
	long	lfl;
	lfl=fabs(fl)+.5;
	if (fl<0) return -lfl;
	return lfl;
}

#ifdef CompXCode


double mypow10(int aPow)
{
	double lRet = 1;
	
	if (aPow > 0) {
		while (aPow--) {
			lRet *= 10;		
		}
	} else if (aPow < 0){
		while (aPow++) {
			lRet /= 10;		
		}
	}
	return lRet;
}

#endif /* CompXCode */


/* GetFieldString kopieert de karakters uit de string aString 
   naar de string aField tot een tab of regeleinde volgt 
   en returnt de lengte van het veld */
int GetFieldString(char *aField, char *aString)
{
	char	*lChar = aString,
			*lField = aField;
	int lLen = 0;
	
	while (*lChar && *lChar != '\t' && *lChar != '\r' && *lChar != '\n') {
		*lField++ = *lChar++;
		lLen++;
	}
	*lField = 0;
	
	return lLen;
}

/* inlezen namen file en geheugen-allocatie									*/
int	LeesNamen()
{
	int		Index;				/* Volgnummer deelnemer of relatie					*/
	char	Nrec[120];
	FILE	*NamenF;
	zdr		lDnrPtr;
	zrl		lRelPtr;
	zmut	lMutaPtr;
	
	NamenF=fopen("namen.dta","r");
	if (NamenF==NULL)
	{
		printf("De namen file: %.12s is helaas niet aanwezig\n","namen.dta");
		printf("Dit programma wordt nu afgebroken\n");
		exit(1);
	}
	
	
	zDn0 = NULL; 
	zRe0 = NULL;
	zMu0 = NULL;
	FuT=BaT=EfT=ObT=ReT=mRel;
	
	sprintf(FuT.Rnm,"FON"); FuT.Idx=500;
	sprintf(BaT.Rnm,"BAN"); BaT.Idx=600;
	sprintf(EfT.Rnm,"EFF"); EfT.Idx=700;
	sprintf(ObT.Rnm,"OBL"); ObT.Idx=800;
	sprintf(ReT.Rnm,"REL"); ReT.Idx=900;
	while (fgets(Nrec,80,NamenF)!=NULL)
	{
		char lField[40];
		int lLen, lPos = 0;
		
		lLen = GetFieldString(lField, Nrec + lPos);
		if (!lLen) {
			exit(-1);
		}
		lPos += lLen + 1; /* naar volgende veld op de regel, plus de komma! */
		
		Index=atoi(lField);
		if (Index > 0) {
			if (Index<500) {
				if (!AddDnr(Index, &lDnrPtr)) printf("Namen.dta: dubbel nummer\n");
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lDnrPtr->Dnm,"%.3s",lField);
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lDnrPtr->Naam,"%.20s",lField);
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (lLen) {
					sprintf(lDnrPtr->Email,"%s",lField); /* email is optioneel */
					lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				} else {
					sprintf(lDnrPtr->Email,"<geen email>"); /* email is optioneel */
				}
				
			} else if (Index < 900) {
				if (!AddRel(Index, &lRelPtr)) printf("Namen.dta: dubbel nummer relatie\n");
				
				lRelPtr->Typ = (Index/100);
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lRelPtr->Rnm,"%.3s",lField);
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lRelPtr->Naam,"%.20s",lField);
				
			} else if (Index < 1000) { /* mutaties */
				if (!ZoekMutaNew(Index, &lMutaPtr)) printf("Namen.dta: dubbel nummer mutatie\n");
								
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lMutaPtr->Rnm,"%.3s",lField);
				
				lLen = GetFieldString(lField, Nrec + lPos);
				if (!lLen) {
					printf("Fout in namen file: %.12s bij regel: %d\n","namen.dta", Index);
					exit(-1);
				}
				lPos += lLen + 1; /* naar volgende veld op de regel, plus de tab! */
				
				sprintf(lMutaPtr->Naam,"%.20s",lField);
				
			}
		}
	}
	fclose(NamenF);
	return 1;
}

