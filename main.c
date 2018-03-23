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
#define TR 20 // number of transactions
#define Balance 5
#define PRINT_VECS 1
#define DEBUG 1
#define MAX_RAND 10000  // max value of elements generated for array


/* Prototypes */
void print_vec(const char *label, double vec[NWallet]);
void print_mat(const char *label, double matrix[NWallet][TR]);
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
	print_vec("Initial Balance", wallets);
	for(int row = 0; row < NWallet ; ++row)
		for(int i = 0; i < TR ; i++){
			amount = random();
			
			if(amount <  wallets[row])
			{
				wallets[row] -= amount;
				transactions[row][i] = -amount;
				#if DEBUG
					printf("Transaction of %f - APPROVED\n", amount);
				#endif
			}else
			{
				transactions[row][i] = 0;
				#if DEBUG
					printf("Transaction of %f - REFUSED\n", amount);
				#endif
			}
		}
	print_vec("Remain Balance", wallets);
	print_mat("Transactions Table", transactions);
	random();
	
	
	return EXIT_SUCCESS;;
}

void print_vec(const char *label, double vec[NWallet]){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i)
		printf("%.2f\t", vec[i]);
	printf("\n\n");
	return;
}

void print_mat(const char *label, double matrix[NWallet][TR]){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i){
		printf("L%i - \t", i);
		for(int j = 0; j< TR; ++j)
			printf("(%.2f)\t", matrix[i][j]);
		printf("\n");
	}
	printf("\n\n");
	return;
}
// Initializa the vector with minimal amount
void initialization(double vec[], int size){
	for(int i =0 ; i< size; i++){
		vec[i] = Balance;
	}
}


float random (){
	double random ;	
	do{
		random = rand() % MAX_RAND;	
	}while(random == 0.0);
	return (random) / MAX_RAND;
	}
