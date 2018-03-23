/* Comp 7926 - Computation Finance Final Project
 * Saulo Quinteiro dos Santos 7845136
 * Daniyal Kowaja
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "omp.h"

/* Constants */

#define NWallet 1 // number of Wallets
#define TR 100 // number of transactions
#define Balance 100
#define PRINT_VECS 1
#define DEBUG 0
#define MAX_RAND 10000  // max value of elements generated for array


/* Prototypes */
void print_vec(const char *label, double *vec);
void print_mat(const char *label, double *matrix);
void initialization(double vec[], int size);
float random();
int main(int argc, char *argv[]){
	//Q1 = Test the function with different parameters.
	//double start, end;
	srand (99);
	double wallets [NWallet];
	double transactions[NWallet][TR];
	double amount; 
	initialization(wallets, NWallet);
	for(int i = 0; i < TR ; i++){
		amount = random();
		
		if(amount <  wallets[0])
		{
			wallets[0] -= amount;
			transactions[0][i] = -amount;
		}else
		{
			transactions[0][i] = 0;			
		}
	}
	random();
	
	
	return EXIT_SUCCESS;;
}

void print_vec(const char *label, double *vec){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i)
		printf("%.2f\t", vec[i]);
	return;
}
void print_mat(const char *label, double *matrix){
}

// Initializa the vector with minimal amount
void initialization(double vec[], int size){
	for(int i =0 ; i< size; i++){
		vec[i] = Balance;
	}
}


float random (){
	double random ;
	int attempt = 0;
	do{
		random = rand() % MAX_RAND;	
		#if DEBUG
		attempt++;
		#endif
	}while(random == 0.0);

	#if DEBUG
		if (attempt > 1)
			printf("Attempt = %i\n", attempt);
		printf("Result = %f\n\n", random / MAX_RAND);
	#endif
	return (random) / MAX_RAND;
	}
