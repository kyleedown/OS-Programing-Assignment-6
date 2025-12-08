#include <stdio.h>
#include<stdlib.h>
#include <errno.h>

typedef struct {
    int8_t page;
    int8_t offset;
} PhysicalAddress;

PhysicalAddress convertAddress(int32_t logicalAddress){
    PhysicalAddress address;
    int16_t masked = logicalAddress & 0xFFFF;
    address.page = (masked >> 8) & 0xFF;
    address.offset = masked & 0xFF;
    
    return address;

}

char *physicalMemory[65536];

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Wrong number of arguments given. This file requires exactly one file name as a command line argument.");
        return -1;
    }

    FILE *fp = NULL;
    char line[10];
    fp = fopen(argv[1],"r");


    if (fp == NULL){
        fprintf(stderr, "Error opening file %s: ", argv[1]);
        perror("");
        return -1;
    }

    printf("File '%s' opened successfully.\n", argv[1]);

    int physicalAddressPage,physicalAddressOffset;

    while(fgets(line,sizeof(line),fp)){
        int32_t logical = atoi(line);
        printf("%d: \n", logical);
        PhysicalAddress pa = convertAddress(logical);
        printf("Page No: %d \n",pa.page);
        printf("Offset: %d \n",pa.offset);

    }


    fclose(fp);
    return 0;

}