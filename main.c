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

#define NWallet 40 // number of Wallets
#define TR 40000 // number of transactions
#define Balance 3
#define PRINT_VECS 1
#define DEBUG 1
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
	double start, end;
	srand (99);
	double wallets [NWallet];
	struct transaction transactions[TR]; //= (Transaction) malloc((NWallet*TR) * sizeof(Transaction));
	double amount;
	int count=0; 
	int sender, receiver;
	int approved=0, refused=0;
	initialization(wallets, NWallet);
	print_vec("Initial Balance", wallets);
	start = omp_get_wtime();
	
	for(int i = 0; i < TR ; i++){
		//for(int sender = 0; sender < NWallet ; ++sender){
			sender = randomI(NWallet); 
			receiver = randomI(sender);
			amount = randomF();
			if (processTransaction(wallets, transactions, sender, receiver, amount, i))
				
				approved++;
			else 
				
				refused++;
			
			count++;
			if(count%100 == 0){
				count = 0;
				usleep(50000); // simulation of proof of work
				savetodisk(wallets, transactions);
				//free(transactions); // flush the values in transaction matrix				
			}
		}			
		
	end = omp_get_wtime();
	#if DEBUG
		print_mat("Transactions Table", transactions);
	#endif
	print_vec("Remain Balance", wallets);
	printf("Simulation of %i Transactions in %f sec\n", NWallet*TR, end-start);
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

void print_mat(const char *label, struct transaction matrix[TR]){
	printf("%s\n", label);
	//for(int i =0; i<NWallet;++i){
	//	printf("L%i-\t", i);
		for(int j = 0; j< TR; ++j)
			printf("%i=>%i=(%.2f)%c;", matrix[j].sender, matrix[j].receiver, matrix[j].amount, matrix[j].status ? 'T':'F');
		printf("\n");
	//}
	//printf("\n\n");
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
				
				
				transactions[i].amount = -amount;
				transactions[i].sender = sender;
				transactions[i].receiver = receiver;
				transactions[i].status = true;				
				#if DEBUG
					printf("Transaction of %f - APPROVED\n", amount);
				#endif
				result = true;
			}else
			{
				transactions[i].amount = 0;
				transactions[i].sender = sender;
				transactions[i].receiver = receiver;
				transactions[i].status = false;	
				
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