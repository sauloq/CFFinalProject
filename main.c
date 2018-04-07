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
#include <stdbool.h>
#include "omp.h"

/* Constants */

#define NWallet 20 // number of Wallets
#define TR 20 // number of transactions
#define Balance 5
#define PRINT_VECS 1
#define DEBUG 0
#define MAX_RAND 10000  // max value of elements generated for array

struct transaction 
{
     int sender;
     int receiver;
     float amount;
	 bool status;
};

/* Prototypes */
void print_vec(const char *label, double vec[NWallet]);
void print_mat(const char *label, struct transaction matrix[NWallet*TR]);
void initialization(double vec[], int size);
float randomF();
int randomI(int exc);
bool flushTDisk(); //fucntion to flush transaction table to the disk
bool flushBDisk(); //fucntion to flush Balance array to the disk
bool processTransaction(double wallets[], struct transaction transactions[NWallet*TR],int sender, int receiver, double amount, int i);
void savetodisk(double wallets[], struct transaction transactions[NWallet*TR]);
int main(int argc, char *argv[]){
	//Q1 = Test the function with different parameters.
	//double start, end;
	srand (time(0));
	double wallets [NWallet];
	struct transaction transactions[NWallet*TR]; //= (Transaction) malloc((NWallet*TR) * sizeof(Transaction));
	double amount;
	int count=0; 
	int sender, receiver;
	int approved=0, refused=0;
	initialization(wallets, NWallet);
	print_vec("Initial Balance", wallets);
	for(int i = 0; i < TR ; i++)
		for(sender = 0; sender < NWallet ; ++sender){
			//sender = randomI();
			receiver = randomI(sender);
			amount = randomF();
			if (processTransaction(wallets, transactions, sender, receiver, amount, i))
				approved++;
			else 
				refused++;
			count++;
			if(count%100 == 0){
				count = 0;
				savetodisk(wallets, transactions);
				//free(transactions); // flush the values in transaction matrix				
			}
		}			
		
	print_vec("Remain Balance", wallets);
	#if DEBUG
	print_mat("Transactions Table", transactions);
	#endif
	printf("Simulation of %i Transactions\n", NWallet*TR);
	printf("Approved = %i\n", approved);
	printf("Refused = %i\n", refused);
	
	
	
	return EXIT_SUCCESS;;
}

void print_vec(const char *label, double vec[NWallet]){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i)
		printf("%.2f\t", vec[i]);
	printf("\n\n");
	return;
}

void print_mat(const char *label, struct transaction matrix[NWallet*TR]){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i){
		printf("L%i - \t", i);
		for(int j = 0; j< TR; ++j)
			printf("%i=>%i=(%.2f) %c\t", matrix[i*NWallet+j].sender, matrix[i*NWallet+j].receiver, matrix[i*NWallet+j].amount, matrix[i*NWallet+j].status ? 'T':'F');
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


float randomF (){
	double random ;	
	do{
		random = rand() % MAX_RAND;	
	}while(random == 0.0);
	return (random) / MAX_RAND;
	}

int randomI (int exc){
	int random ;	
	do{
		random = rand() % NWallet;	
	}while(random == exc); //if sender == receive try other number
	return random;
	}

bool flushTDisk(struct transaction transactions[NWallet*TR]){ //fucntion to flush transaction table to the disk
	return 1;
}
bool flushBDisk(double Wallets[]){ //fucntion to flush Balance array to the disk
	return 1;
}

bool processTransaction(double wallets[], struct transaction transactions[NWallet*TR],int sender, int receiver, double amount, int i){
	bool result;

	if(amount <=  wallets[sender])
			{
				wallets[sender] -= amount;
				wallets[receiver] +=amount;
				
				transactions[sender*NWallet+i].amount = -amount;
				transactions[sender*NWallet+i].sender = sender;
				transactions[sender*NWallet+i].receiver = receiver;
				transactions[sender*NWallet+i].status = true;				
				#if DEBUG
					printf("Transaction of %f - APPROVED\n", amount);
				#endif
				result = true;
			}else
			{
				transactions[sender*NWallet+i].amount = 0;
				transactions[sender*NWallet+i].sender = sender;
				transactions[sender*NWallet+i].receiver = receiver;
				transactions[sender*NWallet+i].status = false;	
				
				#if DEBUG
					printf("Transaction of %f - REFUSED\n", amount);
				#endif
				result = false;
			}
	return result;
}

void savetodisk(double wallets[], struct transaction transactions[NWallet*TR])
{
	flushBDisk(wallets);
	flushTDisk(transactions);
}