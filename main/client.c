#include <sys/types.h>
#include <sys/socket.h>
//send (int sockfd , const void * buf , size_t len , int flags)
//The system calls send() are used to transmit a message to another socket.
//used only when socket is in connected state
 #include <unistd.h>
//read (int sockfd , void * buf , size_t count)
#include <netinet/in.h>
//being used to use structure of sock address
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
const int ITEM = 0;
const int CLOSE = 1;
//The below function converts any char *
//into a string
//Required to convert port number 
//from string to integer
int sockfd , connfd;
int StringToNumber(char * s) {
	int length = strlen(s);
	int i;
	int num = 0;
	for(i = 0 ; i < length ; i++) {
		num *= 10;
		num += s[i] - '0';
	}
	return num;
}
//There is some error there when 
//you delcare buffer as a global variable
//That's why buffer has been declared as 
// a local variable inside the main function
const int N = 1000;
void Handler(int sig) {
	printf("\nClient exiting because of manual break...\n");
	char buf[] = "Termination of Client\n";
	write(sockfd , buf , strlen(buf));
	close(sockfd);
	exit(0);
	return;
}
int main(int argc , char * argv[]) {
	//Fetch IP and port number from terminal
	char * IP = argv[1];
	char * portnumber = argv[2];
	/*Since our portnumber is a string now, we are converting it into
	 *an integer for using it as a port
	*/
	//Port changed to integer from string
	int port = StringToNumber(portnumber);
	struct sockaddr_in serveraddr , client;
	//Socket created for communication
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("Socket Creation Unsuccessful\n"); 
		exit(0); 
	} 
	else {
		printf("Socket Creation was Successful..\n");
	}
	//initializing serveraddr for assigning 
	bzero(&serveraddr , sizeof(serveraddr));
	serveraddr.sin_port = htons(port); 
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); 
	serveraddr.sin_family = AF_INET;
	if (connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) { 
		printf("Connection with the Server failed...\n"); 
		exit(0); 
	}
	else {
		printf("Connected to the server..you may now proceed\n");
	}
	//Main code for Point of Sale Terminal is as follows
	char buffer[N];
	signal(SIGINT , Handler);
	while(1) {
		int request_type;
		printf("Enter your request type\n");
		//Request type is being fetched. It is either 1 or 0 as mentioned in the question
		scanf("%d" , &request_type);
		//Globally defined ITEM = 0 and CLOSE = 1
		if(request_type == ITEM) {
			//string for upc code
			char UPC_Code[64];
			//string for quantity
			char number[10];
			/* Table to be printed here */
			FILE * filepointer;
			filepointer = fopen("data.txt" , "r");
			printf("-------------------------------\n");	
        	printf("| Code |   Item Name   | Rate |\n");
        	printf("-------------------------------\n");
        	char buff[N];
        	int index;
        	int i;
        	while(fgets(buff , sizeof(buff) , filepointer) != NULL) {
		    	buff[strlen(buff) - 1] = '\0';
	    		char upc[64];
	    		char name[N];
	    		char price[10];
	    		//initialization of all string below
	    		bzero(upc , sizeof(upc));
	    		bzero(name , sizeof(name));
	    		bzero(price , sizeof(price));
	    		index = 0;
	    		//This part is being used to fetch the 
	    		//upc code of every line in database
	    		for(i = 0 ; i < strlen(buff) ; i++) {
	    			if(buff[index] == '_') break;
	    			upc[i] = buff[index];
	    			index++;
	    		}
	    		index++;
	    		//This part is being used to fetch the
	    		//name of every line in database
	    		int jj = index;
	    		for(i = jj ; i < strlen(buff) ; i++) {
	    			if(buff[index] == '_') break;
	    			name[i - jj] = buff[index];
	    			index++;
	    		}
	    		index++;
	    		jj = index;
	    		//This part is being used to fetch the
	    		//price of every item in our database
	    		for(i = jj ; i < strlen(buff) ; i++) {
	    			if(buff[index] == '\0') {
	    				break;
	    			}
	    			price[i - jj] = buff[index];
	    			index++;
	    		}
	    		printf("| %-5s|   %-12s| %-5s|\n", upc , name, price);
		    }
		    printf("-------------------------------\n");
		    //filepointer closed for retrieving table
			fclose(filepointer);
			//UPC code and number to be scanned here
			printf("Enter the UPC code of the item from table above\n");
			scanf("%s" , UPC_Code);
			printf("Enter the quantity you want\n");
			scanf("%s" , number);
			//buffer initialized again
			//The reason behind this is that we will be
			//sending this buffer string to socket
			//for passing it to server 
			// for further processes
			bzero(buffer , sizeof(buffer));
			index = 0;
			//buffer string is like this
			//for ex - "type_UPCcode_quantity"
			//if type = 0 and upc code is 123 and quantity is 1
			//my buffer string that will be sent is going to be
			//0_123_1
			buffer[index] = '0';
			index++;
			buffer[index] = '_';
			index++;
			for(i = 0 ; i < strlen(UPC_Code) ; i++) {
				buffer[index] = UPC_Code[i];
				index++;
			}
			buffer[index] = '_';
			index++;
			for(i = 0 ; i < strlen(number) ; i++) {
				buffer[index] = number[i];
				index++;
			}
			//marking the end of the string
			buffer[index] = '\0';
			//write command is being used to send the buffer message
			//to socket which will pass it to server
			write(sockfd , buffer , sizeof(buffer));
			//initailsing the buffer again
			bzero(buffer , sizeof(buffer));
			//this read command will read the message from socket
			//and store it into buffer
			int read_len = read(sockfd , buffer , sizeof(buffer));
			printf("%d\n" , read_len);
			if(read_len == 0) {
				printf("Server was shut down because of manual break. Client also closing\n");
				exit(0);
			}
			//Response from the server is being printed here
			printf("The response from the server is - \n");
			printf("---------------------------------------------------\n");
			printf("%s\n" , buffer);
			printf("---------------------------------------------------\n");
			buffer[0] = '\0';
		}
		//If client decides to close the point of terminal
		else if(request_type == CLOSE) {
			//We will only be sending string = "1"
			//Server will itself understand what to do
			//with this buffer
			buffer[0] = '1';
			buffer[1] = '\0';
			//Writing so that server can read our message
			write(sockfd , buffer , sizeof(buffer));
			//buffer initialized again
			bzero(buffer , sizeof(buffer));
			//now buffer will store what server is sending
			int read_len = read(sockfd , buffer , sizeof(buffer));
			if(read_len == 0) {
				printf("Server was shut down because of manual break. Client also closing\n");
				exit(0);
			}
			printf("The response from the server is - \n");
			printf("---------------------------------------------------\n");
			printf("%s\n" , buffer);
			printf("---------------------------------------------------\n");
			//After the client has entered "1"
			//We will close the following socket id
			close(sockfd);
			printf("The connection has been closed now\n");
			break;
		}
	}
}
