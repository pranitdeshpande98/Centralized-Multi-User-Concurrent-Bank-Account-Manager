# Centralized-Multi-User-Concurrent-Bank-Account-Manager

This project is implemented using TCP Sockets for communication between multiple clients and servers.
 All tests are performed on Ubuntu 22.04.1 (Jammy Jellyfish)
# Working of server- 
 1. The server program is executed with command line arguments specifying the port number and the name of the file which will act as the database on the server i.e., “Records.txt”.
 2.  Socket system calls are called to create a socket, bind the server socket with the client.
 3. The server will first, read the database file in which it stores account number, account name and the current amount.
 4. After reading the file line by line successfully, the server first calls the interest calculation method first by creating a new thread for that function. After calling the interest calculation method mutex lock and unlock are implemented to maintain synchronization and consistency.
 5. The balance amount is updated after the mutex lock according to the rate and time set in the program before unlocking the mutexes.
 6. After interest calculation, the program calls get_data function with the help of threads. Here a new thread will be implemented for each client using pthread_create and each new client request’s connection will be accepted using accept system call, before performing any operation 
on the account mutexes are locked first.
 7. In the get_data method there is a different method perform_bank_operation which will compare the operation type. If the operation type = “d” then, the method will add the balance with the new amount mentioned in the file, if the operation type = “w” then the method will subtract the balance with the new amount mentioned. All the error messages are appropriately handled. Mutexes locks and unlocks are implemented to avoid any race condition.
 8. After operation the server sends back the response to the client and closes the file and socket.
 
 # Working of client-
 1. The client program is executed with command line arguments specifying the host address (127.0.0.1), port number, time stamp and the file name i.e., “Transactions.txt”.
 2. The client will read the file which will have all the details of transactions i.e., the timestamp, account number, operation-to-performed (d – deposit, w – withdraw) and the amount.
 3. Socket is created using socket system call and a new socket connection request is sent to the server. After the server accepts the connection the client sends the transaction data line by line according to the timestamps stored in the file using sleep condition to avoid race issues.
 4. A begin and end clock is set to measure the overall execution time required by each transaction in milliseconds. All the computation happens on the server side. After server performs the operation it sends it data to client.
 5. Additionally, there are two different print (cout) statements which will print the overall execution time required for each transaction and then the avg time required for each client.
 6. The avg time required for each client to complete all the transactions are stored in a file named “file-for-storing-execution-time.txt” during runtime(if file does not exist then it will create the same) which later can be used to create a graph and study scalability issues in it.
 
# Execution steps:
 1. To execute, run the make file 
 2. Type ./server 8000 Records.txt in one terminal and ./client 127.0.0.1 8000 1 Transactions.txt once or ./script.sh for running client 25 times. One can change the number of clients in the script.h file and can run accordingly.
