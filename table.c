#include<stdio.h>
#include<string.h>
#include<stdlib.h>

	int main()
	{
    		FILE *ptr_file;
    		char buf[1000];

    		int num, rate, i, j=0;
    		char item[100];

    		ptr_file =fopen("data.txt","r");
    		if (!ptr_file)
        		{
        			printf("No Data available");
        		}
        	
        	printf("-------------------------------\n");	
        	printf("| Code |   Item Name   | Rate |\n");
        	printf("-------------------------------\n");	
        		for(i=0; i<15; i++)
    		
        		{
        			//printf("%s", buf);
        			fscanf(ptr_file,"%d_%s_%d", &num, item, &rate);
        			printf("|%-6d|%-15s|%-6d|\n",num, item, rate);
        		}
        	printf("-------------------------------\n");


		fclose(ptr_file);
    		return 0;
	}

