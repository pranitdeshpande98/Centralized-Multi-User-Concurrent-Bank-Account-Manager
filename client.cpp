#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <chrono>                  // This header is used to measure execution time for each transaction.
#define MAXELEMENTSIZE 90000      // Here I am defining the maximum transaction size which can be supported by this architecture
using namespace std;

int main(int argc, char *argv[])
{	
	int sockfd, n;
	ifstream transactions(argv[4]);                   //File Name Provided by the user which will have all the transaction details in it.
	if (argc < 5) {                                   //Check whether all the input parameters are entered, if not print the error message
        	 fprintf(stderr,"Error, Please enter hostname, portnumber, timestamp, file path correctly\n");
        	 exit(1);
   	}
   	
	int timestamp[MAXELEMENTSIZE];		
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[MAXELEMENTSIZE];
	int noOfoperations=0;
	int portnumber            = atoi(argv[2]);        //Port Number
	float timestamp_entered   = atof(argv[3]);        //Timestamp entered by user so that i can later compare with the timestamps entered in the transactions file

	float total_time_taken[MAXELEMENTSIZE];	
	string data_line_by_line;
	char * p;
	int i=0;
	string operation[MAXELEMENTSIZE];                    
    	while (getline(transactions, data_line_by_line))        //Here I am reading the data line by line and then calculating the total number of operations.
    	{
// Citation : This code is referred from stackoverflow (stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c)    	
		if(!data_line_by_line.empty()){                
			char str[MAXELEMENTSIZE];
			strcpy(str, data_line_by_line.c_str());
			operation[i]=data_line_by_line;
			p = strtok (str," ");
			timestamp[i]=atoi(p);         //Storing all the timestamps
			noOfoperations++;             // Incrementing the count of all the operations / transactions from the client file
			i++;
		}
	}
	

	sockfd = socket(AF_INET, SOCK_STREAM, 0);    // Step 1 : Socket is created for client
	if (sockfd < 0) {
		cout << "Error in creating a socket";
		exit(1);	
	}
	server = gethostbyname(argv[1]);              //Get host entry associated with a hostname or IP address
	if (server == NULL) {
	        perror("Error please enter correct host\n");
	        exit(1);
	}
	
	int val = 1;	
	
	memset((char *) &serv_addr,0, sizeof(&serv_addr)); // Construct an address for remote server :: zero structure out

	serv_addr.sin_family = AF_INET;
// Citation : This code for creating socket communication is referred from (https://inst.eecs.berkeley.edu/~cs194-5/sp08/lab1/sockets/client.c)
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

	serv_addr.sin_port = htons(portnumber);

	cout << "----------------------------------Connecting Socket------------------------------------ "<< endl;
    	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { // Step 2 : Connect the client socket with the server
        	cout << "Error in connecting the socket";
        	exit(1);
        	}
		
	for(int i=0;i<noOfoperations;i++){
//This piece of code is written so that each transaction will wait for the imestamp entered before it is written to the client.	
//1. if this is the first transaction then sleep the system for timestamp entered by the user * timestamp maintained in the transactions file.
//2. If there are more than one trnasactions then find out the difference between the current timestamp and the timestamp of previous transactions 
// maintained in the file * timestamp maintained in the transactions file.
		if(i==0){
			sleep(timestamp[i]*timestamp_entered);
		}else{
			sleep((timestamp[i] - timestamp[i-1])*timestamp_entered);
	
		}
		bzero(buffer,256);
		strcpy(buffer, operation[i].c_str());//string is converted to char buffer
		
		auto begin = std::chrono::high_resolution_clock::now();		//Start clock time using chrono library
		cout << "Data sent from client to server:" << "\t" << buffer <<endl;
		n = write(sockfd,buffer,strlen(buffer));       //transaction is sent to the server
		if (n < 0) 
			cout << "Unable to write operation to server" << endl;
		bzero(buffer,256);
		n = read(sockfd,buffer,MAXELEMENTSIZE);//response is read from the server
		
		auto end = std::chrono::high_resolution_clock::now();         //End clock time using chrono library
		if (n < 0) 
			cout << "Error reading from socket" << endl;
		        cout << "Client : response received from server :" << buffer << endl;
		 
		
		auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin); //Calculated here total time of execution
		total_time_taken[i]= total_time.count();
		cout<< "Transaction Number:" << i+1 << "\t" << "Execution time is (in microseconds):" << total_time_taken[i] << endl;		
	}
	float avg_time_per_transaction= 0;
	for(int i=0;i<noOfoperations;i++){
		avg_time_per_transaction=(avg_time_per_transaction + total_time_taken[i]) / noOfoperations;// used for calculating average time taken per instruction
	}
	
	transactions.close();
// Print execution time in microseconds and get this data to draw the graph later on. This can be used only when you have single client.	
	cout<< "Average Execution Time:" << avg_time_per_transaction <<"(in microseconds)"  << " " << "for count of transactions: " << noOfoperations <<endl;
	
//This code created file named below and appends the execution time for all the clients with respect to all the transactions.
// This code is used only when you have multiple clients as it becomes very difficult to read all the timestamps from the terminal printed from the above cout statement.
// This below code is used to store the execution time for all the transactions avg per client. This data can be later used to draw the graph.
//This part of commented as it is only for storing time per execution of each transaction.
/*	
	ofstream file1;
	file1.open("file_for_storing_execution_time",ios::app);
	if (!file1.is_open()) {
		cout << "Error in creating file" << endl;
	}
	file1 << avg_time_per_transaction << endl;
	file1.close();
*/	
	close(sockfd);// closed the client socket
	return 0;
}
