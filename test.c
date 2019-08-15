#include <stdio.h>
#include <stdlib.h>
int main() {
	FILE *filepointer;
    filepointer = fopen("data.txt" , "r");
    char buff[50];
    while(fgets(buff , sizeof(buff) , filepointer) != NULL) {
    	buff[strlen(buff) - 1] = '\0';
    	printf("%s\n" , buff);
    }
    char upc[64];
    bzero(upc , sizeof(upc));
    upc[0] = '1';
    printf("%s %d" , upc , strlen(upc));
    return 0;
}