#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <ctime>
#define MAXELEMENTSIZE 90000                        // Here I am defining the maximum transaction size which can be supported by this architecture
using namespace std;

int account_number[MAXELEMENTSIZE], noOfrecords=0;  // account_number will store all the account numbers from server records file
float balance[MAXELEMENTSIZE];                      // balance array will store all the balances from server records file
string account_name[MAXELEMENTSIZE];                // account_name will store all the account names from server records file
fstream record_file;                                // a new file descriptor is created to hold records.txt file i.e.,( file entered by user)
pthread_mutex_t mutex[MAXELEMENTSIZE];              // An array is created for mutexes
void * get_data(void *arg);                         //this is the thread function
string perform_bank_operation(string operation);


void * to_calculate_interest(void *arg){   //This function is created to calculate bank amount interest after every 5 min with the 
	
    while(1){
		sleep(400);
		for(int i=0;i<noOfrecords;i++){
		
			pthread_mutex_lock(&mutex[i]);//acquires the lock when updating each record
			cout << endl << "Lock is currently held successfully by interest method" << endl;
			
			balance[i]=balance[i]*(1 + .15);//every 5 min, interest is added at the rate of 0.15% with interest rate = 3%
			
			pthread_mutex_unlock(&mutex[i]);//releases lock when updated the record
			cout << "Lock is released successfully by interest method" <<endl;
			cout << "Interest amount added successfully:" << balance[i] << " " << "In account Number:" << account_number[i] <<endl;
		
		}
	}
}

string perform_bank_operation(string operation) {
		int size;
		int i=0,fileindex=0;
    		char op[2],temp_string[256];
		string response="";
		
//Citation : This piece of code for tokenizing the string is referred from (https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array)		
		strcpy(temp_string, operation.c_str());//tokenize instruction which is of format = (timestamp accountnumber operation amount)
		strtok (temp_string," ");
		int account=atoi(strtok (NULL, " "));// account variable has the accountnumber from client transaction
		strcpy(op,strtok (NULL, " "));//operation has the operation from client transaction
		float amount=atof(strtok (NULL, " "));//amount has the amount from client transaction
		for(int i=0;i<noOfrecords;i++){
			
			if(account_number[i]==account){

				break;
			}		
			fileindex++;                                                    //fileindex is the index in Records.txt of the accountno sent by client
		}		
		if(fileindex == noOfrecords){
			return "Account number does not exists on the server";
		}
		pthread_mutex_lock(&mutex[fileindex]);                                    //Lock acquired. Below region will be lock.
		
		if(strcmp(op, "d") == 0){     //deposit operation
						
			cout << "Account Number  : " << account_number[fileindex] << "\t\t\t" <<   "Deposit\t\t:"   << amount << endl;
			cout << "Old Balance     : " << balance[fileindex] << "\t\t\t" << "New Balance \t:" << balance[fileindex]+amount << endl;
			balance[fileindex]=balance[fileindex]+amount;								
			response="Balance Deposited Successfully";
		
		}else if(strcmp(op,"w") == 0){            //withdraw operation
			if(balance[fileindex] <= 0 || balance[fileindex]-amount <0){    //check for insufficient balance
			
				cout << "Account Number  : " << account_number[fileindex] << "\t\t\t" <<   "Withdraw\t:"   << amount << endl;
				cout << "Old Balance     : " << balance[fileindex] << "\t\t\t" << "New Balance \t:" << balance[fileindex] << endl;			
				pthread_mutex_unlock(&mutex[fileindex]);
				return "Insufficient balance: cannot withdraw money";
			}
			cout << "Account Number  : " << account_number[fileindex] << "\t\t\t" <<   "Withdraw\t:"   << amount << endl;
			cout << "Old Balance     : " << balance[fileindex] << "\t\t\t" << "New Balance \t:" << balance[fileindex] - amount << endl;	
			balance[fileindex]=balance[fileindex]-amount;		
			response="Balance Withdrawn Successfully";
		}
		else{
			response= "Invalid Operation";
		}	
				
		pthread_mutex_unlock(&mutex[fileindex]);                                // Lock Released. 
		
		return response;
		
}

void * get_data(void *arg){

	int newsockfd=*((int *)arg);
	char buffer[256];
    	bzero(buffer,256);
	string message_to_be_sent ,operation;
	int no_of_operations=0;
	message_to_be_sent = "";
	
    	while(int n = read(newsockfd,buffer,256)){       // Read the data from client.
		if(n <= 0){
			cout << "Error in reading from the socket" << endl;
			exit(1);
		}
		string operation;
		operation.append(buffer, buffer+n);
     		cout << endl << "Data received from client => \t" <<operation << endl;
		bzero(buffer,256);		
		message_to_be_sent = perform_bank_operation(operation);				
		cout << "Server : Response sent to client" << endl;
		no_of_operations++;
		strcpy(buffer,message_to_be_sent.c_str());
     		n = write(newsockfd,buffer,strlen(buffer));   //Write back the response to the client
		if (n < 0){ 
			perror("Error in writing the response back to client");
		}		
	}	
	return 0;
}


int main(int argc, char *argv[])
{
    	int n,i=0,no_of_thread=0;
    	char buffer[MAXELEMENTSIZE], *p,temp_string[22000];			
	string data_line_by_line;
	
    	struct sockaddr_in server_address, client_address;
	for(int i=0;i<MAXELEMENTSIZE;i++){
		pthread_mutex_init(&mutex[i],0);                //In this piece of code mutexes are initialzed to zero.
	}

	if (argc < 3) {
        	 fprintf(stderr,"Please provide port number and record file\n"); // Verify whether the user has entered all the input variables. If not, then print the appropriate error.
        	 exit(1);
   	}

    	record_file.open(argv[2], ios::in);                  //Open the file in read mode and verify whether the file exists in the system, if not then print the error accordingly.
    	if(!record_file.is_open()){
		cout << "Server records file could not be found";
		return 0;
     	}     	

     	int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Step 1 : Create the server socket,and if the returned value is less than 0 print the error.
     	if (sockfd < 0) {
        	cout << "Error in creating server socket" << endl;
        	exit(1);
        	}
        	
       
     	bzero((char *) &server_address, sizeof(server_address));
     	int portnumber = atoi(argv[1]);
// Citation: This code is referred from Advanced Operating System class for creating socket communication in C.     	
     	server_address.sin_family = AF_INET;
     	server_address.sin_addr.s_addr = INADDR_ANY;
     	server_address.sin_port = htons(portnumber);
     	if (bind(sockfd, (struct sockaddr *) &server_address,sizeof(server_address)) < 0) { //Step 2 : Bind the socket, and if the returned value is less than 0 print the error.
	        cout << "Error in binding the socket" << endl;
	        exit(1);
	        }
     	if (listen(sockfd,5) < 0 ) {       //Step 3: Listen to the socket,  and if the returned value is less than 0 print the error.
     		cout << "Error in listening to the socket" <<endl;
     		exit(1);
     			}
     	socklen_t client_length = sizeof(client_address);
     
  	while (getline(record_file, data_line_by_line))               //read record file into their respective arrays
    	{	
// Citation : This code is referred from stackoverflow (stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c)      	
		if(!data_line_by_line.empty()){
		    	strcpy(temp_string, data_line_by_line.c_str());
			account_number[i]=atoi(strtok (temp_string," "));
			p=strtok(NULL, " ");		
			strcpy(temp_string,p); 
			account_name[i]=temp_string;
			balance[i]=atoi(strtok (NULL, " "));
			i++;
			noOfrecords++;	
		} 
		
    	}    
    	
    	record_file.close();
	
	int newsockfd;
	pthread_t thread_1[MAXELEMENTSIZE];
	pthread_t interest_calculator_Thread;  // Create a new thread variable for calculating interest
	if (pthread_create(&interest_calculator_Thread, NULL, &to_calculate_interest, &newsockfd) < 0 ) { 
// Creating a new thread for calculating interest calculation , if the returned value is less than 0, print the error.
		cout << "Error in creating thread invoking interest function" <<endl;
	}	
	while(newsockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_length)){  // Step 4: Accept the incoming client connections from client.cpp
	        no_of_thread++;
	        if (pthread_create(&thread_1[no_of_thread], NULL, &get_data, &newsockfd) < 0 ) { // Create a new thread for each client and check the returned value, if less than 0, then print error.
	        	cout << "Error in creating client thread" <<endl;
	        }
    	}     
	close(newsockfd);
	close(sockfd);
	return 0; 
}
