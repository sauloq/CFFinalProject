/* Comp 7926 - Computation Finance Final Project
 * Saulo Quinteiro dos Santos 7845136
 * Daniyal Kowaja
 * 
 */
#define _BSD_SOURCE // fix the warning using usleep
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "omp.h"
#include <string.h>
#include <unistd.h>

/* Constants */

#define NWallet 40 // number of Wallets
#define TR 1000 // number of transactions
#define Balance 3
#define PRINT_VECS 1
#define DEBUG 0
#define MAX_RAND 10000  // max value of elements generated for array
#define BLOCK "blockchain.log"
#define BLOCKSIZE 100


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
void flushTDisk(); //fucntion to flush transaction table to the disk
void flushBDisk(); //fucntion to flush Balance array to the disk
bool processTransaction(double wallets[], struct transaction transactions[NWallet*TR],int sender, int receiver, double amount, int i);
void savetodisk(double wallets[], struct transaction transactions[NWallet*TR]);
void printChecksum (double vec[]);
int main(int argc, char *argv[]){
	//Q1 = Test the function with different parameters.
	double start, end;
	srand (time(NULL));
	double wallets [NWallet];
	struct transaction transactions[BLOCKSIZE]; //= (Transaction) malloc((NWallet*TR) * sizeof(Transaction));
	double amount;
	int index=0; 
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
			if (processTransaction(wallets, transactions, sender, receiver, amount, index))
				approved++;
			else 
				refused++;
			index++;
			if(index == 100){
				index = 0;
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
	printf("Simulation of %i Transactions in %f sec\n", TR, end-start);
	printf("Approved = %i\n", approved);
	printf("Refused = %i\n", refused);
	printChecksum(wallets);
	
	
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
		for(int j = 0; j< BLOCKSIZE; ++j)
			printf("%i=>%i=(%.2f)%c;", matrix[j].sender, matrix[j].receiver, matrix[j].amount, matrix[j].status ? 'T':'F');
		printf("\n");
	//}
	//printf("\n\n");
	return;
}
// Initializa the vector with minimal amount
void initialization(double vec[], int size){
	int bufsize = (NWallet*9)+1;
	char buf[bufsize];
	char ch;
	FILE *fp = fopen(BLOCK, "r");
	if(!fp){
		for(int i =0 ; i< size; i++){
				vec[i] = Balance;
			}
	}else{
		while((ch = fgetc(fp)) != EOF) {
				if (ch == 'B'){
					ch = fgetc(fp);
					fgets(buf, bufsize, fp);				
				}
			}
			#if DEBUG
				printf("Previous Balance used \n");
			#endif
		if(sizeof(buf) > 0){
			vec[0] = atof(strtok (buf,","));
			for(int i = 1; i < NWallet ; ++i){
				vec[i] = atof(strtok (NULL,","));
			}
		}else{
			for(int i =0 ; i< size; i++){
				vec[i] = Balance;
			}
		}
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

void flushTDisk(struct transaction transactions[NWallet*TR]){ //fucntion to flush transaction table to the disk
	FILE *fp = fopen(BLOCK, "a");
	fprintf(fp, "T;");
	for(int i = 0; i < BLOCKSIZE; i++){
		fprintf(fp, "(%i,%i,%.2f,%c),", transactions[i].sender, transactions[i].receiver, transactions[i].amount, transactions[i].status ? 'T':'F');
	}
		fprintf(fp, "\n");
		fclose(fp);
}
void flushBDisk(double Wallets[]){ //fucntion to flush Balance array to the disk
	FILE *fp = fopen(BLOCK, "a");
		fprintf(fp, "B;");
	for(int i = 0; i < NWallet ; i++){
		fprintf(fp, "%f,", Wallets[i]);
	}
		fprintf(fp, "\n");
		fclose(fp);
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
	flushTDisk(transactions);
	flushBDisk(wallets);
}

void printChecksum(double vec []){
	double sum = 0;
	double check = NWallet*Balance;
	for(int i = 0; i < NWallet; i++){
		sum += vec[i];
	}
	printf("Total balance = %f\n", check);
	printf("Checksum = %f\n", sum);
}

