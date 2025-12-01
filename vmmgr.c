#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Wrong number of arguments given. This file requires exactly one file name as a command line argument.");
        return -1;
    }

    FILE *fp = NULL;
    fp = fopen(argv[1],"r");

    if (fp == NULL){
        fprintf(stderr, "Error opening file %s: ", argv[1]);
        perror("");
        return -1;
    }

    printf("File '%s' opened successfully.\n", argv[1]);

    

    fclose(fp);
    return 0;

}