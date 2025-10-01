#include<stdio.h>
#include<stdlib.h>

int main(int argc,char* argv[]){
	if(argc<2){	// if the argument is missing.
		printf("Error, no argument.\n");	// print error msg
		return 0;				// end the program
	}
	unsigned int addr = atoi(argv[1]);	// convert the argument chars to unsigned int (addr)
	const unsigned int bit = 12;	// as HW4.pdf mentioned. the page size is 4KB (4096 bytes = 2^12 bytes)
	
	// From Operating System Concepts p.360 (9.3.1 Basic Method) we can know
	// address can be divided into two part like this:
	// | Page number	| offset     |
	// | 20bits		| 12 bits    |
	// page number is at upper bit of address.
	// offset is at the lower bit of address
	
	printf("The address %d cotains:\n",addr);	
	printf("page number = %d\n",addr>>bit);   // use right shift to take it. (we can obtain it by addr/4096 also.)	
	printf("offset = %d\n",addr&((1<<bit)-1));// using mask 111111111111 to take the lower 12 bits (we can obtain it by addr%4096 also.)
	return 0;
}
