/*
 * aflos.c -- programma om rente, aflossingsperiode, beginwaarde
 *				eindwaarde of aflossingsbedrag te berekenen.
 * algemene formule:
 * wn = w0 * (1 + i) ** n + u * {(1 + i) ** n - 1} / i
 * wn = waarde lening na n maanden
 * w0 = waarde aan het begin
 * i = maandelijkse interest
 * n = aantal maanden
 * u = maandelijks uitkering (aflossing als waarde negatief is).
 */

#include	<stdio.h>
double	MyPow(double aMantisse, int anExponent);
int GetPeriod(void);
double GetInterest(void);
double GetDouble(char* aString);

double	MyPow(double aMantisse, int anExponent)
{
	int		theSign = 1;
	double	theResult;

	if (anExponent < 0)
	{
		theSign = 0;
		anExponent = -anExponent;
	}

	for (theResult = 1.0; anExponent > 0; anExponent--)
	{
	  theResult *= aMantisse;
	}

	if (!theSign)
	{
		theResult = 1.0 / theResult;
	}

	return (theResult);
}

int GetPeriod(void)
{
	int thePeriod;
	
	printf("Periode (mnd)? ==>");
	scanf("%d", &thePeriod);
	return thePeriod;
}

double GetInterest(void)
{
	double theInterest;
	
	printf("Rente percentage op jaarbasis? ==>");
	scanf("%lf", &theInterest);
	return theInterest * 0.01;
}

double GetDouble(char* aString)
{
	double theDebt;
	
	printf("%s? ==>", aString);
	scanf("%lf", &theDebt);
	return theDebt;
}

int main()
{
	char	theChoice = ' ';
	int		fFirstLine;
	int		theMonth;
	int		theYear;
	int		theNmbrOfPeriods;
	double	thePeriodicPayment; /* = -u ! */
	double	theFactor;
	double	theInterestFactor;
	double	theYearlyInterest;
	double	thePeriodicInterest;
	double	theStartDebt;
	double	theEndDebt;

	double	exp(), fabs(), log(), pow(), sqrt();

	while (theChoice != 'q')
	{
		printf("==>: \n");
		printf("   q(uit)\n");
		printf("   a(flossingsbedrag)\n");
		printf("   b(eginwaarde)\n");
		printf("   e(indwaarde)\n");
		printf("   p(eriode)\n");
		while ((theChoice = getchar()) == '\n')
		{
		}

		switch (theChoice)
		{
		case 'q':
			return 0;
	
		case 'a': /* aflossings bedrag (-u) */
			theNmbrOfPeriods = GetPeriod();
			theYearlyInterest = GetInterest();
			thePeriodicInterest = theYearlyInterest / 12;
			theInterestFactor = 1.0 + thePeriodicInterest;
			theStartDebt = GetDouble("Te lenen bedrag");
			theEndDebt = GetDouble("Schuld aan einde periode");			
			/*
			 * u = (wn - w0 * (i + 1) ** n) * i / {(1 + i) ** n - 1}
			 */
			theFactor = MyPow(theInterestFactor, theNmbrOfPeriods);
			thePeriodicPayment =
				-(theEndDebt - theStartDebt * theFactor) * thePeriodicInterest /
				(theFactor - 1);
			printf("Aflossing = %9.2f\n", thePeriodicPayment);
			break;
	
		case 'b': /* beginwaarde (w0) */
			theNmbrOfPeriods = GetPeriod();
			theYearlyInterest = GetInterest();
			thePeriodicInterest = theYearlyInterest / 12;
			theInterestFactor = 1.0 + thePeriodicInterest;
			theEndDebt = GetDouble("Schuld aan einde periode");
			thePeriodicPayment = GetDouble("Aflossingsbedrag");

			/*
			 * w0 = [wn - u * {(1 + i) ** n - 1} / i] / (1 + i) ** n
			 */
			theFactor = MyPow(theInterestFactor, theNmbrOfPeriods);
			theStartDebt =
				(theEndDebt + thePeriodicPayment * ((theFactor - 1.0) / thePeriodicInterest)) /
				theFactor;
			printf("Te lenen bedrag = %14.2lf\n", theStartDebt);
			break;
	
		case 'e': /* eindbedrag (wn) */
			theNmbrOfPeriods = GetPeriod();
			theYearlyInterest = GetInterest();
			thePeriodicInterest = theYearlyInterest / 12;
			theInterestFactor = 1.0 + thePeriodicInterest;
			theStartDebt = GetDouble("Te lenen bedrag");
			thePeriodicPayment = GetDouble("Aflossingsbedrag");
			/*
			 * wn = w0 * (1 + i) ** n + u * {(1 + i) ** n - 1} / i
			 */
			theFactor = MyPow(theInterestFactor, theNmbrOfPeriods);
			theEndDebt = theStartDebt * theFactor  -
				thePeriodicPayment * ((theFactor - 1) / thePeriodicInterest);
			printf("Schuld na %d maanden = %14.2lf\n", theNmbrOfPeriods, theEndDebt);
			break;
	
		case 'p': /* periode (n) */
			{
				double	theNewEndDebt;
				theYearlyInterest = GetInterest();
				thePeriodicInterest = theYearlyInterest / 12;
				theInterestFactor = 1.0 + thePeriodicInterest;
				theStartDebt = GetDouble("Te lenen bedrag");
				theEndDebt = GetDouble("Schuld aan einde periode");
				thePeriodicPayment = GetDouble("Aflossingsbedrag");
				/*
				 * first order approximation
				 * n = (wn - w0) / (w0 * i + u)
				 */
				/*
				theNmbrOfPeriods = (theEndDebt - theStartDebt) /
					(theStartDebt * thePeriodicInterest - thePeriodicPayment);
				do
				{
					theNmbrOfPeriods--;
					theFactor = MyPow(theInterestFactor, theNmbrOfPeriods);
					theNewEndDebt = theStartDebt * theFactor  -
						thePeriodicPayment * ((theFactor - 1) / thePeriodicInterest);
				} while (theNewEndDebt < theEndDebt && theNmbrOfPeriods >= 0);
				*/
				/*
				 * n = log((wn + u/i) / (w0 + u/i)) / log(1 + 1)
				 */
				theFactor = -thePeriodicPayment / thePeriodicInterest;
				theNmbrOfPeriods = log((theEndDebt + theFactor) / (theStartDebt + theFactor)) /
					log(1.0 + thePeriodicInterest);
			}
			printf("Aantal maanden = %d\n", ++theNmbrOfPeriods);
			break;
	
		case 'r': /* rente percentage */
			/*
			theNmbrOfPeriods = GetPeriod();
			theYearlyInterest = GetInterest();
			thePeriodicInterest = theYearlyInterest / 12;
			theInterestFactor = 1.0 + thePeriodicInterest;
			theStartDebt = GetDouble("Te lenen bedrag");
			theEndDebt = GetDouble("Schuld aan einde periode");
			thePeriodicPayment = GetDouble("Aflossingsbedrag");
			if (thePeriodicPayment == 0.0) {
			  theYearlyInterest = (pow(theEndDebt / theStartDebt, (double)1.0 / theNmbrOfPeriods) - 1.0) * 100.0;
			}
			else {
				double t1, t2, t3, t4, t5;
	
				t1 = -theNmbrOfPeriods * (theStartDebt - (theNmbrOfPeriods - 1) * .5 * thePeriodicPayment);
				t2 = t1 * t1;
				t3 = -theNmbrOfPeriods * (theNmbrOfPeriods - 1) * (theStartDebt * (thePeriodicPayment / 3.0) * (theNmbrOfPeriods - 2));
				t4 = 2 * t3 * (theStartDebt - thePeriodicPayment * theNmbrOfPeriods - theEndDebt);
				theYearlyInterest = (t1 - sqrt(t2 - t4)) / t3;
				do
				{
				  theFactor = MyPow(1.0 + theYearlyInterest, -theNmbrOfPeriods);
				  t1 = theStartDebt - (thePeriodicPayment * (1.0 - theFactor) / theYearlyInterest) - theEndDebt * theFactor;
				  t2 = (-thePeriodicPayment / thePeriodicInterest) * (theNmbrOfPeriods * theFactor / (1.0 + thePeriodicInterest * 0.01) - (1.0 - theFactor) / thePeriodicInterest);
				  t3 = theEndDebt * theNmbrOfPeriods * theFactor / (1.0 + thePeriodicInterest * 0.01);
				  t4 = t1 / (t2 + t3);
				  thePeriodicInterest -= t4;
				} while ((float)(fabs(t4)) >= 0.001);
			}
			printf("Rente percentage op jaarbasis = %7.2lf\n",
			  thePeriodicInterest * 100.0);
			*/
			break;
	
		default:
			printf("Onbekende keuze\n");
			break;
		}
	
		printf("Foef aflossingslijst? [y,n] ==>");

		while ((theChoice = getchar()) == '\n')
		{
		}

		if (theChoice == 'y')
		{
			printf("Jaar,maand? ==>");
			scanf("%d,%d", &theYear, &theMonth);
			printf("Duur =                          %8d mnd\n",
				theNmbrOfPeriods);
			printf("Rente percentage op jaarbasis = %8.2f%%\n",
				theYearlyInterest * 100.0);
			printf("Te lenen bedrag =              Û%8.2f\n",
				theStartDebt);
			printf("Aflossingsbedrag =             Û%8.2f\n",
				thePeriodicPayment);
			printf("Jaar\t31 maart\t30 juni\t30 september\t31 december\n");

			fFirstLine = 1;
			do
			{
				printf("%4d", theYear);

				do
				{
					if (fFirstLine)
					{
						int k;
		
						for (k = 1; k < theMonth; k+=3)
						{
							printf("\t\t");

						}
						fFirstLine = 0;
					}
					if (!(theMonth % 3))
					{
						if (theStartDebt <= (double)0.0)
						{
							printf("\t\tnihil");

						}
						else
						{
							printf("\tÛ\t%8.2f", theStartDebt);

						}
					}
					theStartDebt += (theStartDebt * thePeriodicInterest) - thePeriodicPayment;
				} while (++theMonth <= 12);
				theMonth = 1;
				theYear++;

				printf( "\n");

			} while (theStartDebt > theEndDebt);
		}
	}
	return 0;
}

