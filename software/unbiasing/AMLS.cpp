// AMLS.cpp : Defines the entry point for the console application.
//

/* 
AMLS

Convert a biased uncorrelated bitstream into 
an unbiased uncorrelated bitstream.

This source is based on an algorithm called 
"Advanced Multilevel Strategy"
described in a paper titled "Tossing a Biased Coin"
by Michael Mitzenmacher
http://www.fas.harvard.edu/~libcs124/CS/coinflip3.pdf
*/

/* 
   Each input byte must be a zero or a one. 
   Other inputs will cause invalid output 
*/

/* The output is ascii zeroes and ones */
/* The length of the output is less than or equal to the output */
/* Output array deallocation is the responsibility of the caller */


/* 
To facilitate storage this recursive implementation does not process 
the bits In the same order as described in the paper.  It does not 
produce exactly the same output, but the output will be unbiased if 
the input is truly uncorrelated.
*/


#include "stdafx.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

#define BIAS_PERCENT    90   /* Percentage of one's in the input */
#define INPUT_SIZE   10000   /* Number of bits in the input */
#define ITERATIONS     100   /* Number of iterations in the test */


char * OutBits;
int OutCount;
char * InBits  ;

/* Process input stream to yield output bits and two additional biased streams */
void amls_round( int start, int end) 
{
	if (start >= end ) return ;
	int doubles = start;
	int IndexLow  = start;
	int IndexHigh = end;


	do{
		if (InBits[IndexLow] == InBits[IndexHigh]) 
		{	
			InBits[doubles++] = InBits[IndexLow];
			InBits[IndexHigh] = '0'; 
		}else{
			OutBits[OutCount++] = InBits[IndexLow];
			InBits[IndexHigh] = '1';
		}	
		IndexHigh--;
		IndexLow++;
	}while (IndexHigh>IndexLow);
/* Process the two derived streams */
/* Remove both recursive calls to implement Von Neuman algorithm.*/
/* Remove second recursive call to implement unadvanced MLS */
	amls_round(start,--doubles);  
	amls_round(++IndexHigh,end);
	return ;
}


/* AMLS algorithm implementation */
char * AMLS(char *BitStream) 
{
	int InputCount = strlen(BitStream);
	InBits  = (char *) malloc(InputCount+1);
	OutBits = (char *) malloc(InputCount+1);
	strcpy(InBits, BitStream);
	OutCount =  0;
  	amls_round( 0, InputCount-1) ;
	OutBits[OutCount] = '\0';
  	free(InBits);
	return OutBits ;
}



/* Example and test */
int main(int argc, char* argv[]) 
{
	int n= INPUT_SIZE;   
	int i;
	char * TestResults;
	char TestInput[INPUT_SIZE+1];
	for (int j =0 ; j < ITERATIONS ; j++)   
	{
		srand(time(0)+j);
		for ( i=0 ; i<n ; i++ ) 
		{
			TestInput[i] = (rand()%100 < BIAS_PERCENT) ? '1':'0';
		}
		TestInput[i] = 0;   /* set the end byte */
		TestResults = AMLS(TestInput);
		printf("\n%d", strlen(TestResults));
	}
  	printf("\nOutput = %s", TestResults);  /* Show last output */
	printf("\nTest   = %s", TestInput);    /* Show last input */
	free(TestResults);                     /* Caller's responsibility */
	return 0;
}

