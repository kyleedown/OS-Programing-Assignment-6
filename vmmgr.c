// Virtual Memory Manager
// Implements logical-to-physical address translation using:
// - Page Table
// - TLB with LRU replacement
// - Demand paging from BACKING_STORE.bin
//
// NOTE: This version supports 256 physical frames (no page replacement).


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
} VirtualAddress;

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

// Masks the lower 16 bits of the logical address and extracts:
// - page number (upper 8 bits)
// - offset (lower 8 bits)
VirtualAddress convertAddress(int32_t logicalAddress){
    VirtualAddress address;
    int16_t masked = logicalAddress & 0xFFFF;
    address.page = (masked >> 8) & 0xFF;
    address.offset = masked & 0xFF;
    
    return address;

}

// Searches the TLB for a given page number.
// If found (TLB hit), updates LRU timestamp and returns frame number.
// Otherwise returns -1 to indicate a TLB miss.
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

// Inserts a (page → frame) mapping into the TLB.
// First looks for a free entry; if none exists, replaces the
// least recently used (LRU) entry and updates timestamps.
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

// Handles a page fault by loading the requested page from
// BACKING_STORE.bin into the next available physical frame.
//
// Steps:
// 1. Seek to the correct offset in the backing store.
// 2. Read one full 256-byte page into physical memory.
// 3. Warn if fewer than 256 bytes are read.
// 4. Update the page table and frame-to-page mapping.
// 5. Return assigned frame number.
int loadPage(int page, FILE *backing) {
    

    pageFaults++;
    int frame = nextFreeFrame;
    nextFreeFrame++;


    fseek(backing, page * FRAME_SIZE, SEEK_SET);
    // fread(physicalMemory[frame], 1, FRAME_SIZE, backing);

    size_t bytes = fread(physicalMemory[frame], 1, FRAME_SIZE, backing);
    if (bytes < FRAME_SIZE) {
    fprintf(stderr,
        "Warning: Expected to read %d bytes but only read %zu bytes "
        "from BACKING_STORE.bin (page %d). File may be corrupted.\n",
        FRAME_SIZE, bytes, page
    );
    }

    pageTable[page] = frame;
    frameToPage[frame] = page;


    return frame;
}




int main(int argc, char *argv[]){

    // Verify correct number of command-line arguments.
    // The program expects exactly one argument: the addresses file.
    if(argc != 2){
        printf("Wrong number of arguments given. This file requires exactly one file name as a command line argument.");
        return -1;
    }

    // Attempt to open the input file containing logical addresses.
    // Exit on failure with a descriptive message.
    FILE *fp = NULL;
    char line[10];
    fp = fopen(argv[1], "r");


    if(fp == NULL) {
        fprintf(stderr, "Error opening file %s: ", argv[1]);
        perror("");
        return -1;
    }

    // printf("File '%s' opened successfully.\n", argv[1]);

    // Mark all page table entries as invalid (-1).
    // This indicates that no pages are currently loaded in memory.
    for (int i = 0; i < PAGE_COUNT; i++) {
        pageTable[i] = -1;
    }

    nextFreeFrame = 0;
    pageFaults = 0;
    totalAddresses = 0;

    // Clear all TLB entries.
    // page = -1 means the entry is unused.
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page = -1;
        tlb[i].frame = -1;
        tlb[i].lastUsed = 0;
    }

    // Open BACKING_STORE.bin in binary mode.
    // This file acts as the disk where pages are stored.
    // Each page is exactly 256 bytes.
    FILE *backing = fopen("BACKING_STORE.bin","rb");
    if (!backing) {
        perror("Error opening BACKING_STORE.bin");
        return -1;
    }


    // Read each logical address from the input file.
    // For each address:
    //   1. Convert to page number and offset.
    //   2. Look up in TLB.
    //   3. If TLB miss → consult page table.
    //   4. If page table miss → page fault → load page from backing store.
    //   5. Insert mapping into TLB.
    //   6. Calculate physical address and fetch signed byte.
    //   7. Print required output.
    while(fgets(line,sizeof(line),fp)) {
        uint32_t logical = atoi(line);
        VirtualAddress va = convertAddress(logical);
        totalAddresses++;

        int frame = tlbLookup(va.page);

        if(frame == -1){
            frame = pageTable[va.page];
        
            if(frame == -1){

                frame = loadPage(va.page,backing);
            }

            tlbInsert(va.page, frame);

        }

        int physicalAddress = frame * FRAME_SIZE + va.offset;

        signed char value = physicalMemory[frame][va.offset];

        
        printf("Logical: %u Physical: %d Value: %d\n",
       logical, physicalAddress, (int)value);

        }

    // Output final statistics:
    // - Page Fault Rate
    // - TLB Hit Rate
    printf("------------------------------------\nPage Fault Rate = %.3f\n", (float)pageFaults / totalAddresses);
    printf("TLB Hit Rate = %.3f\n", (float)tlbHits / totalAddresses);

    // Close all opened files and exit.
    fclose(backing);
    fclose(fp);
    return 0; 

}