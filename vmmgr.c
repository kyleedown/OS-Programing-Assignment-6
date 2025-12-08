#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


#define PAGE_COUNT 256
#define FRAME_SIZE 256
#define TLB_SIZE 16
#define PHYSICAL_FRAMES 256

typedef struct {
    uint8_t page;
    uint8_t offset;
} PhysicalAddress;

typedef struct {
    int page;
    int frame;
    int lastUsed;
} TLBEntry;

TLBEntry tlb[TLB_SIZE];
int pageTable[PAGE_COUNT];
signed char physicalMemory[PHYSICAL_FRAMES][FRAME_SIZE];
int frameToPage[PHYSICAL_FRAMES];
int nextFreeFrame = 0;
int pageFaults = 0;
int tlbHits = 0;
int totalAddresses = 0;
int timeCounter = 0;

PhysicalAddress convertAddress(uint32_t logicalAddress){
    PhysicalAddress address;
    uint16_t masked = logicalAddress & 0xFFFF;
    address.page = (masked >> 8) & 0xFF;
    address.offset = masked & 0xFF;
    return address;

}

int tlbLookup(int page) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].page == page) {
            tlb[i].lastUsed = timeCounter++;
            tlbHits++;
        return tlb[i].frame;
        }
    }
    return -1;
}

void tlbInsert(int page, int frame) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].page == -1) {
            tlb[i].page = page;
            tlb[i].frame = frame;
            tlb[i].lastUsed = timeCounter++;
        return;
        }
    }
    int lru = 0;
    for (int i = 1; i < TLB_SIZE; i++) {
        if (tlb[i].lastUsed < tlb[lru].lastUsed) lru = i;
    }
    tlb[lru].page = page;
    tlb[lru].frame = frame;
    tlb[lru].lastUsed = timeCounter++;
}

int loadPage(int page, FILE *backing) {
    pageFaults++;
    int frame = nextFreeFrame;
    nextFreeFrame++;


    fseek(backing, page * FRAME_SIZE, SEEK_SET);
    fread(physicalMemory[frame], 1, FRAME_SIZE, backing);


    pageTable[page] = frame;
    frameToPage[frame] = page;


    return frame;
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Wrong number of arguments given. This file requires exactly one file name as a command line argument.");
        return -1;
    }

    FILE *fp = NULL;
    char line[10];
    fp = fopen(argv[1], "r");


    if(fp == NULL) {
        fprintf(stderr, "Error opening file %s: ", argv[1]);
        perror("");
        return -1;
    }

    printf("File '%s' opened successfully.\n", argv[1]);

    int physicalAddressPage,physicalAddressOffset;

    while(fgets(line,sizeof(line),fp)) {
        uint32_t logical = atoi(line);
        printf("%d: ", logical);
        PhysicalAddress pa = convertAddress(logical);
        printf("%d, %d\n", pa.page,pa.offset);
    }

    fclose(fp);
    return 0; 

}