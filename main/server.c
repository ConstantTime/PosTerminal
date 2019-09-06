#include <stdio.h>
//For using listen after the bind section
#include <sys/types.h>        
#include <sys/socket.h>
// Listen can be used by including the above 2 files
#include <arpa/inet.h>    //close  
#include <netinet/in.h>  
#include <sys/time.h> //for macros
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>  
#include <signal.h>
//The below function converts any char *
//into a string
//Required to convert port number 
//from string to integer
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
const int N = 1024;
//MAX is defined to support the max number of clients
const int MAX = 30;
//SIZE is defined to store the size of all arrays 
const int SIZE = 123;
int client_socket[123];
//Bill stores the total amount of shopping every client does
int bill[123];
int opt = 1;
int master_sockfd;
int addrlen;
int port;
int i;
int new_socket;
//Signal Handler designed as explicitly told in the question
void Handler(int sig) {
	signal(SIGINT , Handler);
	int i;
	for(i = 0 ; i < MAX ; i++) {
		if(client_socket[i] != 0) {
			printf("\nSocket number - %d is being closed.\n" , client_socket[i]);
			close(client_socket[i]);
			client_socket[i] = 0;
			bill[i] = 0;
		}
	}
	close(master_sockfd);
}
//The below function converts any 
//positive integer into a string
char * IntegerToString(int b) {
	static char ans[123];
	bzero(ans , sizeof(ans));
	int i = 0;
	int a = b;
	while(a != 0) {
		int temp = a % 10;
		ans[i] = '0' + temp;
		a = a / 10;
		i++;
	}
	static char ans2[123];
	bzero(ans2 , sizeof(ans2));
	int len = strlen(ans);
	int index = 0;
	for(i = len - 1 ; i >= 0 ; i--) {
		ans2[index] = ans[i];
		index++;
	}
	return ans2;
}
// Driver function
int main(int argc , char * argv[]) { 
	struct sockaddr_in serveraddr; 
	//If arguments are less than 2, server gives an error
	//To remind the user that he has to pass the portnumber also
    if(argc > 2) {
        printf("Number of arguments entered by you exceeds the max limit\n");
        printf("Server is exiting now because of your mistake\n");
        exit(0);
    }
    if(argc < 2) {
        printf("Number of arguments entered by you is less than what should have been provided\n");
        printf("Server is exiting now because of your mistake\n");
        exit(0);   
    }
    signal(SIGINT , Handler);
    char * portnumber = argv[1];
    //port number being changed from string to integer
    port = StringToNumber(portnumber);
    //Socket Creation Function
    fd_set readfds;
    for(i = 0 ; i < MAX ; i++) {
        client_socket[i] = 0;
    }
    master_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //SOCK_STREAM for TCP connection and SOCK_DGRAM for UPD connection 
    if (master_sockfd == 0) { 
        printf("Socket creation has failed\n"); 
        exit(0); 
    } 
    else {
        printf("Socket has been successfully created\n"); 
    }
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if(setsockopt(master_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 ) {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }  
    // assign IP, PORT 
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port); 
    // Binding newly created socket to given IP and verification 
    if ((bind(master_sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))) != 0) { 
        printf("Socket bind failed.\n");
    }

    if (listen(master_sockfd, MAX) < 0) {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
    //accept the incoming connection  
    addrlen = sizeof(serveraddr);   
    puts("Waiting for connections ...");        
    while(1) {    
        int sd;
        int max_sd;
        //To clear the socket set
        FD_ZERO(&readfds);        
        //Master socket added to the set 
        FD_SET(master_sockfd, &readfds);   
        max_sd = master_sockfd;     
        //Child sockets added to the set
        for (i = 0 ; i < MAX ; i++) {
            //socket descriptor  
            sd = client_socket[i];                    
            //if valid socket descriptor then add to read list  
            if(sd > 0) {
                FD_SET( sd , &readfds);
            }
            //Max value of sd needed to work with select function  
            if(sd > max_sd) {
                max_sd = sd;   
            }
        }        
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        int activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);   
        if ((activity < 0) && (errno!=EINTR)) {   
            printf("select error");   
        }
        //If something happened on the master socket ,  
        //then its an incoming connection 
        char buffer[N]; 
        if(FD_ISSET(master_sockfd, &readfds)) {        	        	
            if ((new_socket = accept(master_sockfd , (struct sockaddr *)&serveraddr , (socklen_t*)&addrlen))<0) {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }                
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(serveraddr.sin_addr) , ntohs(serveraddr.sin_port));   
            //send new connection greeting message
            //buffer being initalized
            bzero(buffer , sizeof(buffer));
            //data read from client
            int input_len = read(new_socket , buffer , N);
         // DEBUG - printf("%d %s\n" , input_len , buffer);
            if(input_len == 0 || strcmp(buffer ,  "Termination of Client\n") == 0) {
            	printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(serveraddr.sin_addr) , ntohs(serveraddr.sin_port)); 
            	close(new_socket);
            	continue;
            }
            //As told in the client.c file 
            //type was the first character of buffer
            char type = buffer[0];
            char UPC[64];
    		char NAME[N];
    		char PRICE[10];
    		char upc_code[64];
           	char number[10];
           	//if user is buying an item
           	//the following is being done at a time
            //when user is giving first request
            if(type == '0') {
            	int index = 0;
            	bzero(upc_code , sizeof(upc_code));
            	bzero(number , sizeof(number));
            	//the following loops fetch the upc code and quantity
            	//from the buffer read through client
            	for(i = 2 ; i < strlen(buffer) ; i++) {
            		if(buffer[i] == '_') break;
            		upc_code[index] = buffer[i];
            		index++;
            	}
            	int upc_end = i;
            	index = 0;
            	for(i = upc_end + 1 ; i < strlen(buffer) ; i++) {
            		if(buffer[i] == '\0') break;
            		number[index] = buffer[i];
            		index++;
            	}
            	//Now we will check protocols and errors
            	//First checking whether quantity is a positive integer or not
            	int quantity_is_number_or_not = 1;
            	for(i = 0 ; i < strlen(number) ; i++) {
            		if(number[i] < '0' || number[i] > '9') {
            			quantity_is_number_or_not = 0; //0 means false
            		}
            	}
            	//reinitializing the buffer again for sending data to client
            	bzero(buffer , sizeof(buffer));
            	if(quantity_is_number_or_not == 0) {
            		char str[] = "Response - 1 \nNumber of items should be a definite positive integer\n";
            		//Response being stored to buffer
            		for(i = 0 ; i < strlen(str) ; i++) {
            			buffer[i] = str[i];
            		}
            	}
            	else {
            		//Now we will check whether upc code is found in database or not
            		FILE * filepointer;
            		filepointer = fopen("data.txt" , "r");
            		char buff[N];
            		int upc_code_found = 0;
            		//fgets command fetches each line from data.txt and stores it in buff
            		//EACH LINE OF data.txt LOOKS LIKE THIS 
            		//UPCCODE_NAME_PRICE
            		while(fgets(buff , sizeof(buff) , filepointer) != NULL) {
				    	buff[strlen(buff) - 1] = '\0';
			    		char upc[64];
			    		char name[N];
			    		char price[10];
			    		bzero(upc , sizeof(upc));
			    		bzero(name , sizeof(name));
			    		bzero(price , sizeof(price));
			    		//buff stores the string in each line of data.txt
			    		//our task is to separate upc , name and price from that string
			    		index = 0;
			    		//fetching upc from buff
			    		for(i = 0 ; i < strlen(buff) ; i++) {
			    			if(buff[index] == '_') break;
			    			upc[i] = buff[index];
			    			index++;
			    		}
			    		index++;
			    		int jj = index;
			    		//fetching name from buff
			    		for(i = jj ; i < strlen(buff) ; i++) {
			    			if(buff[index] == '_') break;
			    			name[i - jj] = buff[index];
			    			index++;
			    		}
			    		index++;
			    		jj = index;
			    		//fetching price from buff
			    		for(i = jj ; i < strlen(buff) ; i++) {
			    			if(buff[index] == '\0') {
			    				break;
			    			}
			    			price[i - jj] = buff[index];
			    			index++;
			    		}
			    		//if upc in any line comes out to same as upc_code that
			    		//we fetched from buffer then okay..otherwise there is a problem
			    		if(strcmp(upc_code , upc) == 0) {
			    			//if one line found with same upc, set upc_code_found to 1
			    			upc_code_found = 1;
			    			bzero(NAME , sizeof(NAME));
			    			bzero(UPC , sizeof(UPC));
			    			bzero(PRICE , sizeof(PRICE));
			    			//Storing the name of this product where UPC matched for future use
			    			for(i = 0 ; i < strlen(name) ; i++) {
			    				NAME[i] = name[i];
			    			}
			    			//Storing the price of this product where UPC matched for future use
			    			for(i = 0 ; i < strlen(price) ; i++) {
			    				PRICE[i] = price[i];
			    			}
			    			//Storing the UPC of this product where UPC matched for future use
			    			for(i = 0 ; i < strlen(upc) ; i++) {
			    				UPC[i] = upc[i];
			    			}
			    			//I don't need to search any other line becuase upc got matched in this line itself
			    			//Also, UPC codes that we have used are unique
			    			//Acting as primary key in our database
			    			break;
			    		}
				    }
				    //Point to note - if not found, then error
				    bzero(buffer , sizeof(buffer));
				    if(upc_code_found == 0) {
				    	/*When UPC is not there is database
				    	we have two choices -
				    	1. either go with continuing the session of client
				    	2. or shut down the client side
				    	We have chosen then option number - 1 in our code
				    	*/
				    	char str[] = "Response - 1 \nUPC not found in database\n";
	            		//Store this response in buffer
	            		for(i = 0 ; i < strlen(str) ; i++) {
	            			buffer[i] = str[i];
	            		}
				    }
				    else {
				    	//Case when there is no problem now 
				    	//Now in this case 
				    	char str[] = "Response - 0 \nPrice of item is ";
				    	//I will add the price in str
				    	int j = strlen(str);
				    	//Price being added as was saved above for future
				    	for(i = 0 ; i < strlen(PRICE) ; i++) {
				    		str[j] = PRICE[i];
				    		j++;
				    	}
				    	char str2[] = " and Name is ";
				    	j = strlen(str);
				    	for(i = 0 ; i < strlen(str2) ; i++) {
				    		str[j] = str2[i];
				    		j++;
				    	}
				    	j = strlen(str);
				    	//Name being added as was saved above for future
				    	for(i = 0 ; i < strlen(NAME) ; i++) {
				    		str[j] = NAME[i];
				    		j++;
				    	}
				    	//Our final string will look something like this 
				    	//Response - 0 Price of item is __price__ and Name is __name__
				    	//stroing it in buffer
				    	for(i = 0 ; i < strlen(str) ; i++) {
				    		buffer[i] = str[i];
				    	}
				    }
            		fclose(filepointer);
            	}
            	//sending response to client
	            write(new_socket , buffer , strlen(buffer));            
	            //add new socket to array of sockets  
	            for (i = 0; i < MAX; i++) {
	                //if position is empty  
	                if(client_socket[i] == 0) {  
	                	//client socket registered
	                    client_socket[i] = new_socket;   
	                    //price of item changed to integer
	                    int price_of_item = StringToNumber(PRICE);
	                    //quantity changed to integer
	                    int quantity = StringToNumber(number);
	                    //Bill[i] stored the bill of this client
	                    bill[i] += price_of_item * quantity;
	                   // printf("%d %d\n" ,price_of_item , bill[i]);
	                    break;   
	                }   
	            }  
	        } // end of type - 0
	        else if(type == '1') {
	        	//Since it was the start of communication
	        	//Bill will be 0 as this client has not 
	        	//purchased anything
	        	bzero(buffer , sizeof(buffer));
	        	char str[] = "Response - 0 \nYour bill is - 0";
	        	for(i = 0 ; i < strlen(str) ; i++) {
	        		buffer[i] = str[i];
	        	}
	        	//sending data to client in case of type - 1
	        	write(new_socket , buffer , strlen(buffer));
	        }
	        puts("Response sent to client successfully");
	    }   
	    int id;
        //else its some IO operation on some other socket 
        for (id = 0; id < MAX; id++) {   
            sd = client_socket[id];
            if (FD_ISSET(sd , &readfds)) {   
                //Check if it was for closing , and also read the  
                //incoming message  
                bzero(buffer , sizeof(buffer));
                int valread = read(sd , buffer, 1024);
                if (valread == 0) {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&serveraddr , (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(serveraddr.sin_addr) , ntohs(serveraddr.sin_port));                            
                    //Close the socket and mark as 0 in list for reuse  
                    close(sd);   
                    client_socket[id] = 0;   
                    bill[id] = 0;
                }                     
                //Echo back the message that came in  
                else {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';
                    printf("Current id is %d\n" , client_socket[id]);
                    //As told in the client.c file 
		            //type was the first character of buffer
		            char type = buffer[0];
		            char UPC[64];
		    		char NAME[N];
		    		char PRICE[10];
		    		char upc_code[64];
		           	char number[10];
		            if(type == '0') {
		            	int index = 0;
		            	bzero(upc_code , sizeof(upc_code));
		            	bzero(number , sizeof(number));
		            	//the following loops fetch the upc code and quantity
		            	//from the buffer read through client
		            	for(i = 2 ; i < strlen(buffer) ; i++) {
		            		if(buffer[i] == '_') break;
		            		upc_code[index] = buffer[i];
		            		index++;
		            	}
		            	int upc_end = i;
		            	index = 0;
		            	for(i = upc_end + 1 ; i < strlen(buffer) ; i++) {
		            		if(buffer[i] == '\0') break;
		            		number[index] = buffer[i];
		            		index++;
		            	}
		            	//Now we will check protocols and errors
		            	//First checking whether quantity is a positive integer or not
		            	int quantity_is_number_or_not = 1;
		            	for(i = 0 ; i < strlen(number) ; i++) {
		            		if(number[i] < '0' || number[i] > '9') {
		            			quantity_is_number_or_not = 0; //0 means false
		            		}
		            	}
		            	//reinitializing the buffer again for sending data to client
		            	bzero(buffer , sizeof(buffer));
		            	if(quantity_is_number_or_not == 0) {
		            		char str[] = "Response - 1 \nNumber of items should be a definite positive integer\n";
		            		//Response being stored to buffer
		            		for(i = 0 ; i < strlen(str) ; i++) {
		            			buffer[i] = str[i];
		            		}
		            	}
		            	else {
		            		//Now we will check whether upc code is found in database or not
		            		FILE * filepointer;
		            		filepointer = fopen("data.txt" , "r");
		            		char buff[N];
		            		int upc_code_found = 0;
		            		//fgets command fetches each line from data.txt and stores it in buff
		            		//EACH LINE OF data.txt LOOKS LIKE THIS 
		            		//UPCCODE_NAME_PRICE
		            		while(fgets(buff , sizeof(buff) , filepointer) != NULL) {
						    	buff[strlen(buff) - 1] = '\0';
					    		char upc[64];
					    		char name[N];
					    		char price[10];
					    		bzero(upc , sizeof(upc));
					    		bzero(name , sizeof(name));
					    		bzero(price , sizeof(price));
					    		//buff stores the string in each line of data.txt
					    		//our task is to separate upc , name and price from that string
					    		index = 0;
					    		//fetching upc from buff
					    		for(i = 0 ; i < strlen(buff) ; i++) {
					    			if(buff[index] == '_') break;
					    			upc[i] = buff[index];
					    			index++;
					    		}
					    		index++;
					    		int jj = index;
					    		//fetching name from buff
					    		for(i = jj ; i < strlen(buff) ; i++) {
					    			if(buff[index] == '_') break;
					    			name[i - jj] = buff[index];
					    			index++;
					    		}
					    		index++;
					    		jj = index;
					    		//fetching price from buff
					    		for(i = jj ; i < strlen(buff) ; i++) {
					    			if(buff[index] == '\0') {
					    				break;
					    			}
					    			price[i - jj] = buff[index];
					    			index++;
					    		}
					    		//if upc in any line comes out to same as upc_code that
					    		//we fetched from buffer then okay..otherwise there is a problem
					    		if(strcmp(upc_code , upc) == 0) {
					    			//if one line found with same upc, set upc_code_found to 1
					    			upc_code_found = 1;
					    			bzero(NAME , sizeof(NAME));
					    			bzero(UPC , sizeof(UPC));
					    			bzero(PRICE , sizeof(PRICE));
					    			//Storing the name of this product where UPC matched for future use
					    			for(i = 0 ; i < strlen(name) ; i++) {
					    				NAME[i] = name[i];
					    			}
					    			//Storing the price of this product where UPC matched for future use
					    			for(i = 0 ; i < strlen(price) ; i++) {
					    				PRICE[i] = price[i];
					    			}
					    			//Storing the UPC of this product where UPC matched for future use
					    			for(i = 0 ; i < strlen(upc) ; i++) {
					    				UPC[i] = upc[i];
					    			}
					    			//I don't need to search any other line becuase upc got matched in this line itself
					    			//Also, UPC codes that we have used are unique
					    			//Acting as primary key in our database
					    			break;
					    		}
						    }
						    //Point to note - if not found, then error
						    bzero(buffer , sizeof(buffer));
						    if(upc_code_found == 0) {
						    	/*When UPC is not there is database
						    	we have two choices -
						    	1. either go with continuing the session of client
						    	2. or shut down the client side
						    	We have chosen then option number - 1 in our code
						    	*/
						    	char str[] = "Response - 1 \nUPC not found in database\n";
			            		//Store this response in buffer
			            		for(i = 0 ; i < strlen(str) ; i++) {
			            			buffer[i] = str[i];
			            		}
						    	int end_ = strlen(buffer);
						    	buffer[end_] = '\0';
						    }
						    else {
						    	//Case when there is no problem now 
						    	//Now in this case 
						    	char str[] = "Response - 0 \nPrice of item is ";
						    	//I will add the price in str
						    	int j = strlen(str);
						    	//Price being added as was saved above for future
						    	for(i = 0 ; i < strlen(PRICE) ; i++) {
						    		str[j] = PRICE[i];
						    		j++;
						    	}
						    	char str2[] = " and Name is ";
						    	j = strlen(str);
						    	for(i = 0 ; i < strlen(str2) ; i++) {
						    		str[j] = str2[i];
						    		j++;
						    	}
						    	j = strlen(str);
						    	//Name being added as was saved above for future
						    	for(i = 0 ; i < strlen(NAME) ; i++) {
						    		str[j] = NAME[i];
						    		j++;
						    	}
						    	//Our final string will look something like this 
						    	//Response - 0 Price of item is __price__ and Name is __name__
						    	//stroing it in buffer
						    	for(i = 0 ; i < strlen(str) ; i++) {
						    		buffer[i] = str[i];
						    	}
						    	int end_ = strlen(buffer);
						    	buffer[end_] = '\0';
						    }
		            		fclose(filepointer);
			            	//sending response to client
				            write(sd , buffer , strlen(buffer));            
				            //add new socket to array of sockets  
				            int price_of_item = StringToNumber(PRICE);
		                    //quantity changed to integer
		                    int quantity = StringToNumber(number);
		                    //Bill[i] stored the bill of this client
		                    bill[id] += price_of_item * quantity;
		            	}
	                   // printf("%d %d\n" ,price_of_item , bill[i]); 
			        } // end of type - 0
			        else if(type == '1') {
			        	char str[] = "Response - 0 \nYour bill is - ";
			        	//my resultant string that will be 
			        	//sent to the client is going to be
			        	//Response - 0 Your bill is - __bill__
			        	bzero(buffer , sizeof(buffer));
			        	for(i = 0 ; i < strlen(str) ; i++) {
			        		buffer[i] = str[i];
			        	}
			        	int buffer_end = strlen(buffer);
			        	//bill[id] changed to string to add it into our buffer
			        	char * str2 = IntegerToString(bill[id]);
			        	for(i = 0 ; i < strlen(str2) ; i++) {
			        		buffer[buffer_end] = str2[i];
			        		buffer_end++;
			        	}
			        	//buffer set to the string as told above
			        	//buffer sent to client
			        	write(sd , buffer , strlen(buffer));/*
			        	if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)){   
			                perror("send");   
			            }*/
			        }
			        puts("Response sent to client successfully");   
                }   
            }   
        }   
    }   
}
