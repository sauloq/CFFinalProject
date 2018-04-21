/* Comp 7926 - Computation Finance Final Project
 * Saulo Quinteiro dos Santos 7845136
 * Daniyal Kowaja 
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
#include <memory.h>
#include "sha256.h"
#include "sha256.c"
#include "assert.h"

/* Constants */
#define OpenMP 0//Using of openmp threads to enhace the performance on the simulations of blocks' loop;
#define OpenMP1 0 //Using of openmp threads to enhace the performance on the PoW();
#define ThreadNum 50
#define NWallet 1000 // number of Wallets
//#define TR 10000 //00 // number of transactions
#define PoWD 1000000 //Range of number to find; 10 millions
#define BLOCK 2// number of blocks on the simulation
#define BLOCKSIZE 10000
#define Balance 3
#define PRINT_VECS 0
#define DEBUG 0
#define MAX_RAND 1000000  // max value of elements generated for array
#define FILENAME "blockchain.log"

//Global variable store the previous block hash
BYTE Phash[SHA256_BLOCK_SIZE];
double initbalance[NWallet];

//Struct to store transactions informations
struct transaction 
{
     int sender;
     int receiver;
     float amount;
	 bool status;
};

/* Prototypes */
void print_vec(const char *label, double vec[NWallet]); // print Vectors
void print_mat(const char *label, struct transaction matrix[BLOCKSIZE]); //Print Transactions
void initialization(double vec[]); // initialize the balance, based on a blockchain file or initial values
float randomF(); // Generate random amount
int randomI(int exc); // Generate random integer with exception
void flushTDisk(struct transaction transactions[BLOCKSIZE]); //fucntion to flush transaction table to the disk
void flushBDisk(double Wallets[]); //fucntion to flush Balance array to the disk
void flushHDisk(BYTE hash[SHA256_BLOCK_SIZE], int nonce); // Flush the hash to the disk
// function that verify balance to approve transactions and update the balances
bool processTransaction(double wallets[], struct transaction transactions[BLOCKSIZE],int sender, int receiver, double amount, int i); 
void savetodisk(double wallets[], struct transaction transactions[BLOCKSIZE], int nonce); // Save information to the disk
void printChecksum (double vec[]); // Check if the amount of money in the simulation still the same after the simulation
void hashbalance(double Wallets[],BYTE hash[SHA256_BLOCK_SIZE]); // Generate a hash of the balance array
void hashblock(double Wallets[], BYTE previous[SHA256_BLOCK_SIZE], BYTE newhash[SHA256_BLOCK_SIZE]); // Hash the balance array with the previous hash
bool checkBlockChain(); //process the entire blockchain file, verifying the consistence of the blockchain (BUG)
void simulation();// Simulate X transactions between N entities
bool CheckPoW(int try, int key); // check if the nonce achieve the requirements of difficult
int PoW ();//return the nonce
// Tools function to manipulate hexadecimal
BYTE convertStringToByte( const char * str ); 
BYTE convertCharToByte( const char ch );


// Receive parameter to simulate 's' or check the blockchain integrity 'c' in case of no parameter it runs the simulation and Check process
int main(int argc, char *argv[]){
	
	if (argc == 2){
		switch(argv[1][0]){
			case 's':
				simulation();
				break;
			case 'c':
				checkBlockChain();
				break;
			default:
				simulation();
				checkBlockChain();
		}
	}else{
		simulation();
		checkBlockChain();
	}
}
//Simulation function
void simulation(){
	double start, end;	
	double powstart, powend, powtotal = 0;
	double filestart, fileend, filetotal = 0;
	srand (time(NULL));
	double wallets [NWallet];
	struct transaction transactions[BLOCKSIZE]; 
	double amount;
	int sender, receiver;
	int approved=0;
	#if OpenMP1 || OpenMP
	printf("Simulation using %i OpenMP Threads\n", ThreadNum);
	#pragma omp single
	#endif
	initialization(wallets);
	print_vec("Initial Balance", wallets);
	start = omp_get_wtime();
	#if OpenMP
		#pragma omp parallel for reduction (+:approved) reduction(+:powtotal) reduction(+:filetotal) private (sender, receiver, amount, powend, powstart, filestart, fileend) shared(wallets, transactions) num_threads(ThreadNum)
		#endif
	for (int b = 0; b < BLOCK;++b){
		for(int i = 0; i < BLOCKSIZE ; i++){
			sender = randomI(NWallet); 
			receiver = randomI(sender);
			amount = randomF();
			if (processTransaction(wallets, transactions, sender, receiver, amount, i))
				approved++;			
		}
		powstart = omp_get_wtime();
		PoW(); // simulation of proof of work
		powend = omp_get_wtime();
		powtotal += (powend - powstart);
		/*filestart =  omp_get_wtime();
		savetodisk(wallets, transactions, 1);	
		fileend = omp_get_wtime();
		filetotal += (fileend - filestart);		*/
	}
		
	end = omp_get_wtime();
	#if DEBUG
		//print_mat("Transactions Table", transactions);
	#endif 
	print_vec("Remain Balance", wallets);
	printf("Simulation with %i Wallets, Block= %i, Blocksize = %i and ", NWallet,BLOCK, BLOCKSIZE);
	printf("Transactions= %i\n", BLOCK*BLOCKSIZE);
	printf("Time PoW = %f \t FILE = %f \t Simulation time = %f \n", powtotal/BLOCK, filetotal/BLOCK ,end - start);
	printf("Approved/Refused = %i/%i \t ratio= %f\n", approved, (BLOCK*BLOCKSIZE)-approved, (double)approved/(BLOCK*BLOCKSIZE) );
	printChecksum(wallets);
}

void print_vec(const char *label, double vec[NWallet]){
	printf("%s\n", label);
	for(int i =0; i<NWallet;++i)
		printf("%.2f\t", vec[i]);
	printf("\n\n");
}

void print_mat(const char *label, struct transaction matrix[]){
	printf("%s\n", label);
		for(int j = 0; j< BLOCKSIZE; ++j)
			printf("%i=>%i=(%.2f)%c;", matrix[j].sender, matrix[j].receiver, matrix[j].amount, matrix[j].status ? 'T':'F');
		printf("\n");
}
// Initialize the vector with balance from the file or minimal amount
void initialization(double vec[]){
	int bufsize = (NWallet*10)+1;
	char buf[bufsize];
	char ch;
	char hashbuf[SHA256_BLOCK_SIZE*2+1];
	BYTE hash[SHA256_BLOCK_SIZE];
	int nonce=0;
	if(checkBlockChain()){
		sleep(1);
		FILE *fp = fopen(FILENAME, "r");
		if(!fp){
			printf("File problem to open\n");
			for(int i =0 ; i< NWallet; i++){
					vec[i] = Balance;
					initbalance[i] = Balance;
				}
			flushBDisk(vec);
			hashbalance(vec,Phash);
			hashblock(vec,Phash,hash);
			flushHDisk(hash, nonce);
		}else{
			while((ch = fgetc(fp)) != EOF) {
					if(ch == 'B'){
						ch = fgetc(fp);
						fgets(buf, bufsize, fp);	

					}else if (ch == 'H'){
						ch = fgetc(fp);
						fgets(hashbuf, SHA256_BLOCK_SIZE*2+1, fp);
					}
				}
			#if DEBUG
				printf("Previous Balance used \n");
			#endif
			if(sizeof(buf) > 0 && sizeof(hashbuf)> 0){
				vec[0] = atof(strtok (buf,","));
				for(int i = 1; i < NWallet ; ++i){
					vec[i] = atof(strtok (NULL,","));
					initbalance[i] = vec[i];
				}

				char * ptr = hashbuf;
						int index= 0;
						while(*ptr!='\0'){
							Phash[index++]= convertStringToByte(ptr);
							ptr+=2;
						}
			}
		}
	}else{
		printf("Generating new blockchain\n");
		remove(FILENAME);
		for(int i =0 ; i< NWallet; i++){
				vec[i] = Balance;
				initbalance[i] = Balance;
			}
		flushBDisk(vec);
		hashbalance(vec,Phash);
		hashblock(vec,Phash,hash);
		flushHDisk(hash, nonce);
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

void savetodisk(double wallets[], struct transaction transactions[BLOCKSIZE], int nonce)
{	
	#if OpenMP
	#pragma omp critical (File_handler)
	{
	#endif
		BYTE hash[SHA256_BLOCK_SIZE];
		flushTDisk(transactions);
		flushBDisk(wallets);
		hashblock(wallets, Phash ,hash);
		flushHDisk(hash, nonce);
	#if OpenMP
	}
	#endif
}
//fucntion to flush transaction table to the disk
void flushTDisk(struct transaction transactions[BLOCKSIZE]){ 
	FILE *fp = fopen(FILENAME, "a");
	fprintf(fp, "T;");
	for(int i = 0; i < BLOCKSIZE; i++){
		fprintf(fp, "(%i,%i,%f,%c),", transactions[i].sender, transactions[i].receiver, transactions[i].amount, transactions[i].status ? 'T':'F');
	}
		fprintf(fp, "\n");
		fclose(fp);
}

//fucntion to flush Balance array to the disk
void flushBDisk(double Wallets[]){ 
	FILE *fp = fopen(FILENAME, "a");
		fprintf(fp, "B;");
	for(int i = 0; i < NWallet ; i++){
		fprintf(fp, "%f,", Wallets[i]);
	}
		fprintf(fp, "\n");
		fclose(fp);
}

// function that flush hash to the disk
void flushHDisk(BYTE hash[SHA256_BLOCK_SIZE],int nonce){ 
	memcpy(Phash, hash, SHA256_BLOCK_SIZE);
	FILE *fp = fopen(FILENAME, "a");
	fprintf(fp,"N;%i\n",nonce);
	fprintf(fp, "H;");
	for(int i = 0 ; i<SHA256_BLOCK_SIZE;i++)
		fprintf(fp,"%02x", Phash[i]);
	fprintf(fp, "\n\n");
	fclose(fp);
}

bool processTransaction(double wallets[], struct transaction transactions[BLOCKSIZE],int sender, int receiver, double amount, int i){
	bool result;

	if(amount <=  wallets[sender])
			{
				#if OpenMP
				#pragma omp atomic
				#endif
				wallets[sender] -= amount;
				#if OpenMP
				#pragma omp atomic
				#endif
				wallets[receiver] +=amount;		

				transactions[i].amount = -amount;
				transactions[i].sender = sender;
				transactions[i].receiver = receiver;
				transactions[i].status = true;				
				#if PRINT_VECS
					printf("Transaction of %f - APPROVED\n", amount);
				#endif
				result = true;
			}else
			{
				transactions[i].amount = 0;
				transactions[i].sender = sender;
				transactions[i].receiver = receiver;
				transactions[i].status = false;	
				
				#if PRINT_VECS
					printf("Transaction of %f - REFUSED\n", amount);
				#endif
				result = false;
			}
	return result;
}

void printChecksum(double vec []){
	double sum = 0;
	double check = NWallet*Balance;
	for(int i = 0; i < NWallet; i++){
		sum += vec[i];
	}
	printf("Total balance = %.4f\n", check);
	printf("Checksum = %.4f\n", sum);
}

void printHash(BYTE hash[SHA256_BLOCK_SIZE]){
	for(int i = 0 ; i<SHA256_BLOCK_SIZE;i++)
		printf("%02x", hash[i]);
	printf("\n");
}

void hashbalance(double Wallets[], BYTE hash[SHA256_BLOCK_SIZE]){
	SHA256_CTX ctx;
	BYTE balance[9];
	sha256_init(&ctx);
	for(int i = 0 ; i < NWallet ; i++){
		snprintf(balance, 9, "%f", Wallets[i]);
		sha256_update(&ctx, balance, sizeof(balance));
	}
	sha256_final(&ctx, hash);
	#if DEBUG
		printHash(hash);
	#endif
}

void hashblock(double Wallets[], BYTE previous[SHA256_BLOCK_SIZE], BYTE newhash[SHA256_BLOCK_SIZE]){
	SHA256_CTX ctx;
	BYTE balanceh[SHA256_BLOCK_SIZE];
	hashbalance(Wallets, balanceh);
	sha256_init(&ctx);
	sha256_update(&ctx, previous, sizeof(previous));
	sha256_update(&ctx, balanceh, sizeof(balanceh));	
	sha256_final(&ctx, newhash);
	#if DEBUG
		//printHash(newhash);
	#endif
}

int PoW (){
	int key = rand() % PoWD;	
	int flagPoW =1;
	int try = 0;
	BYTE text1[] = {"NONCE"};
	BYTE result[SHA256_BLOCK_SIZE];
	#if OpenMP1
	int chunk = PoWD/ThreadNum;
	#pragma omp parallel shared(flagPoW) num_threads(ThreadNum) private (try)
	{
		int mynum = omp_get_thread_num();
		try = mynum * chunk;	
	#endif
		do{
			SHA256_CTX ctx;
			BYTE solution[SHA256_BLOCK_SIZE];
			sha256_init(&ctx);
			sha256_update(&ctx, Phash, sizeof(Phash));
			sha256_update(&ctx, text1, sizeof(text1));	
			sha256_final(&ctx, solution);
			if(CheckPoW(try,key)){
				flagPoW = 0;
				memcpy(result, solution, SHA256_BLOCK_SIZE);
				//printf("k=%i\n", try);
			}
			try++;
		}while(flagPoW == 1);
	#if OpenMP1
	}
	#endif
	return key;
}


//Function that simulate the checking of the proof of work
bool CheckPoW(int test, int nonce){
	if(test == nonce)
		return true;
	return false;
}

BYTE convertCharToByte( const char ch )
{
	BYTE value =0;
	if( '0'<=ch&&ch<='9'){
		value=ch-'0';
	}
	else if('A'<=ch&&ch<='F')
	{
		value=(ch-'A')+10;
	}
	else if('a' <=ch&&ch<='f')
	{
		value=(ch-'a')+10;
	}
	else
	{
		//error
	}
	return value;
}

BYTE convertStringToByte( const char * str )
{
	BYTE value=0;
	assert( str);
    if(str[0]=='\0'){
		//empty
	}
    else if(str[1]=='\0'){
		value=convertCharToByte(str[0]);
	}
	else{
		value=(convertCharToByte(str[0]) << 4) |convertCharToByte(str[1]);
	}
	return value;
}

bool checkBlockChain(){
	
	double vec[NWallet];
	int bufsize = (NWallet*10)+2;
	char buf[bufsize];
	char hashbuf[SHA256_BLOCK_SIZE*2+1];
	BYTE Fhash[SHA256_BLOCK_SIZE]; // File Hash
	BYTE Chash[SHA256_BLOCK_SIZE]; // Computed Hash
	char ch;
	SHA256_CTX ctx;
	int pass = 1;
	int flag = 0;

	FILE *fp = fopen(FILENAME, "r");
	if(!fp){
		printf("Empty Block Chain\n");
	}else{
		sha256_init(&ctx);
		while((ch = fgetc(fp)) != EOF) {
			if(ch == 'B'){
				ch = fgetc(fp);
				fgets(buf, bufsize, fp);	
				if(sizeof(buf) > 0){
					vec[0] = atof(strtok (buf,","));
					for(int i = 1; i < NWallet ; ++i){
						vec[i] = atof(strtok (NULL,","));		
					}
				}
				hashblock(vec, Phash, Chash);
				if (!flag){
					flag = 1;
					hashbalance(vec, Phash);
					hashblock(vec,Phash,Chash);
				}
			}else if (ch == 'H'){
				ch = fgetc(fp);
				fgets(hashbuf, SHA256_BLOCK_SIZE*2+1, fp);
				char * ptr = hashbuf;
				int index= 0;
				while(*ptr!='\0'){
					Fhash[index++]= convertStringToByte(ptr);
					ptr+=2;
				}
				memcpy(Phash, Fhash, SHA256_BLOCK_SIZE);
				#if DEBUG
					printf("c-");
					printHash(Chash);
					printf("f-");
					printHash(Fhash);
				#endif
				pass = pass && !memcmp(Fhash, Chash, SHA256_BLOCK_SIZE);
				if(!pass)
				{
					#if DEBUG
						printf("P-");
						printHash(Phash);
						printf("C-");
						printHash(Chash);
						printf("F-");
						printHash(Fhash);
						printf("F-%s\n", hashbuf);
					#endif
					break;
				}								
			}				
		}
		if (pass){
			printf("Blockchain is working fine\n");
			return true;
		}else{
			printf("Blockchain is corrupted\n");
		}		
	}
	return false;
}
