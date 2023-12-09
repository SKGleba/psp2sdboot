#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>

#define BLOCK_SIZE 0x200
#define ALIGN_SECTOR(s) ((s + (BLOCK_SIZE - 1)) & -BLOCK_SIZE) // align (arg) to BLOCK_SIZE

uint32_t getSz(const char* src) {
    FILE* fp = fopen(src, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0L, SEEK_END);
    uint32_t sz = ftell(fp);
    fclose(fp);
    return sz;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("\nusage: %s [code_blob] [output]\n\n", argv[0]);
        return -1;
    }
    
    uint32_t code_sz = getSz(argv[1]);
    if (!code_sz) {
        printf("code doesnt exist or is empty!\n");
        return -1;
    }

    void* full_va = calloc(1, 0x20000);
    if (!full_va) {
        printf("could not alloc 0x%X!\n", 0x20000);
        return -1;
    }

    //*(uint32_t*)(full_va + 0x30) = -1;
    //*(uint32_t*)(full_va + 0x34) = -1;

    *(uint32_t*)(full_va + 0x30) = 1;
    *(uint32_t*)(full_va + 0x34) = 0xDE; // max
    //*(uint32_t*)(full_va + 0x34) = (ALIGN_SECTOR(code_sz) / BLOCK_SIZE);

    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("could not open for read!\n");
        free(full_va);
        return -1;
    }
    fread(full_va + BLOCK_SIZE, 1, code_sz, fp);
    fclose(fp);

    FILE* fw = fopen(argv[2], "wb");
    if (fw == NULL) {
        printf("could not open for write!\n");
        free(full_va);
        return -1;
    }
    fwrite(full_va, 1, 0x20000, fw);
    fclose(fw);

    free(full_va);

    printf("all done, code [0x%X | 0x%X] @ %s(1)\n", code_sz, (ALIGN_SECTOR(code_sz) / BLOCK_SIZE), argv[2]);
}