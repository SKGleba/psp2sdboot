#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>

#define BLOCK_SIZE 0x200
#define MAX_PAYLOAD_BLOCKS 0xDE
#define ADDCONT_SIZE 0x20000 // additional payloads/whatever
#define ALIGN_SECTOR(s) ((s + (BLOCK_SIZE - 1)) & -BLOCK_SIZE) // align (arg) to BLOCK_SIZE
#define MBR_SL_OFFSZ 0x30
#define DEFAULT_OUTPUT "fake.bin"
#define DEFAULT_STAGE2_DEST 0x40000
#define STAGE1_STAGE2_PARAMBUF 0x34 // hardcoded offset,dest,size for stage2
#define STAGE1_EXTRAS_PARAMARR 0x90 // variable-length array of offset,dest,size for additional files

uint32_t getSz(const char* src) {
    FILE* fp = fopen(src, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0L, SEEK_END);
    uint32_t sz = ftell(fp);
    fclose(fp);
    return sz;
}

int injectBytes(const char* file, uint32_t offset, const void* data, uint32_t size) {
    printf("INJECT 0x%X to %s @ 0x%X\n", size, file, offset);
    FILE* fp = fopen(file, "rb+");
    if (fp == NULL) {
        printf("could not open %s for read/write!\n", file);
        return -1;
    }
    fseek(fp, offset, SEEK_SET);
    fwrite(data, 1, size, fp);
    fclose(fp);
    return 0;
}

int appendBytes(const char* file, const void* data, uint32_t size) {
    printf("APPEND 0x%X to %s at 0x%X\n", size, file, getSz(file));
    FILE* fp = fopen(file, "ab");
    if (fp == NULL) {
        printf("could not open %s for append!\n", file);
        return -1;
    }
    fwrite(data, 1, size, fp);
    fclose(fp);
    return 0;
}

int readBytes(const char* file, uint32_t offset, void* data, uint32_t size) {
    printf("READ 0x%X from %s @ 0x%X\n", size, file, offset);
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        printf("could not open %s for read!\n", file);
        return -1;
    }
    fseek(fp, offset, SEEK_SET);
    fread(data, 1, size, fp);
    fclose(fp);
    return 0;
}

int copyFile(const char* src, const char* dst, uint32_t size) {
    if (!size)
        size = getSz(src);
    printf("COPY %s to %s [0x%X]\n", src, dst, size);
    void *data = malloc(size);
    if (!data) {
        printf("could not alloc 0x%X!\n", size);
        return -1;
    }
    FILE* src_fp = fopen(src, "rb");
    if (src_fp == NULL) {
        printf("could not open %s for read!\n", src);
        return -1;
    }
    fread(data, 1, size, src_fp);
    fclose(src_fp);
    FILE* dst_fp = fopen(dst, "wb");
    if (dst_fp == NULL) {
        printf("could not open %s for write!\n", dst);
        return -1;
    }
    fwrite(data, 1, size, dst_fp);
    fclose(dst_fp);
    free(data);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s [PAYLOAD] <-o OUTPUT> <-ia BASE|-ir BASE> <-a FILE> <-s FILE DEST>\n", argv[0]);
        printf("  [PAYLOAD]   - payload path\n");
        printf("  -o OUTPUT   - output path\n");
        printf("  -ir BASE    - replace existing\n");
        printf("  -ia BASE    - append after base, inject header\n");
        printf("  -a FILE     - append file\n");
        printf("  -s FILE     - append file and set its params in stage1\n");
        printf("Before adding/injecting/appending, each file is padded to 0x%X\n\n", ADDCONT_SIZE);
        return -1;
    }

    // parse args
    char *output = DEFAULT_OUTPUT;
    char *base = NULL;
    char *appends[16];
    uint32_t appends_dest[16];
    memset(appends, 0, sizeof(appends));
    memset(appends_dest, 0, sizeof(appends_dest));
    int append_count = 0;
    int replace = 0;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-o")) {
            if (i + 1 < argc) {
                output = argv[i + 1];
                i++;
            }
        } else if (!strcmp(argv[i], "-ir")) {
            if (i + 1 < argc) {
                base = argv[i + 1];
                replace = 1;
                i++;
            }
        } else if (!strcmp(argv[i], "-ia")) {
            if (i + 1 < argc) {
                base = argv[i + 1];
                i++;
            }
        } else if (!strcmp(argv[i], "-a")) {
            if (i + 1 < argc) {
                appends[append_count] = argv[i + 1];
                append_count++;
                i++;
            }
        } else if (!strcmp(argv[i], "-s")) {
            if (i + 1 < argc) {
                appends[append_count] = argv[i + 1];
                // check if there is a dest offset given, should start with 0x
                if (i + 2 < argc) {
                    if (argv[i + 2][0] == '0' && argv[i + 2][1] == 'x') {
                        appends_dest[append_count] = strtol(argv[i + 2], NULL, 16);
                        i++;
                    }
                }
                if (!appends_dest[append_count])
                    appends_dest[append_count] = DEFAULT_STAGE2_DEST;
                append_count++;
                i++;
            }
        }
    }

    if (!argv[1]) {
        printf("no input set!\n");
        return -1;
    }
    
    uint32_t code_sz = getSz(argv[1]);
    if (!code_sz || (code_sz > (MAX_PAYLOAD_BLOCKS * BLOCK_SIZE))) {
        printf("invalid payload size!\n");
        return -1;
    }

    uint32_t payload_loc[2];  // [0] = offset, [1] = size [in blocks]
    payload_loc[0] = 1;
    // payload_loc[0] = -1; // crash
    payload_loc[1] = MAX_PAYLOAD_BLOCKS;
    // payload_loc[1] = (ALIGN_SECTOR(code_sz) / BLOCK_SIZE);

    if (base) {
        uint32_t base_sz = getSz(base);
        if (!base_sz) {
            printf("invalid base size!\n");
            return -1;
        }
        base_sz = ALIGN_SECTOR(base_sz);
        if (copyFile(base, output, base_sz))
            return -1;
        if (replace) {
            if (readBytes(output, 0x30, payload_loc, 8))
                return -1;
            if ((payload_loc[0] > (base_sz / BLOCK_SIZE)) || ((payload_loc[0] + payload_loc[1]) > (base_sz / BLOCK_SIZE))) {
                printf("invalid payload location!\n");
                return -1;
            }
        } else
            payload_loc[0] = (base_sz / BLOCK_SIZE);
    }

    void* full_va = malloc(ADDCONT_SIZE);
    if (!full_va) {
        printf("could not alloc 0x%X!\n", ADDCONT_SIZE);
        return -1;
    }

    // create/replace the payload
    int ret = 0;
    memset(full_va, 0, ADDCONT_SIZE);
    memcpy(full_va + 0x30, payload_loc, 8);
    if (readBytes(argv[1], 0, full_va + BLOCK_SIZE, code_sz))
        return -1;
    if (base) {
        if (injectBytes(output, 0x30, payload_loc, 8))
            return -1;
        if (replace)
            ret = injectBytes(output, payload_loc[0] * BLOCK_SIZE, full_va + BLOCK_SIZE, (payload_loc[1] * BLOCK_SIZE));
        else
            ret = appendBytes(output, full_va + BLOCK_SIZE, (payload_loc[1] * BLOCK_SIZE));
    } else
        ret = appendBytes(output, full_va, ADDCONT_SIZE);
    if (ret)
        return -1;

    // append additional files
    uint32_t s1load_params[3]; // img_offset (blocks), memory_dest, size (blocks)
    for (int i = 0; i < append_count; i++) {
        memset(full_va, 0, ADDCONT_SIZE);
        uint32_t sz = getSz(appends[i]);
        if (!sz) {
            printf("invalid append %s size!\n", appends[i]);
            return -1;
        }
        if (sz > ADDCONT_SIZE) {
            printf("append %s too big!\n", appends[i]);
            return -1;
        }
        if (readBytes(appends[i], 0, full_va, sz))
            return -1;
        if (appends_dest[i]) {
            if (getSz(output) % BLOCK_SIZE) {
                printf("cannot append stage2 to non-block-aligned image!\n");
                return -1;
            }
            s1load_params[0] = getSz(output) / BLOCK_SIZE;
            s1load_params[1] = appends_dest[i];
            s1load_params[2] = (ALIGN_SECTOR(sz) / BLOCK_SIZE);
            if (s1load_params[1] == DEFAULT_STAGE2_DEST) {
                if (injectBytes(output, (payload_loc[0] * BLOCK_SIZE) + STAGE1_STAGE2_PARAMBUF, s1load_params, 0xC))
                    return -1;
            } else {
                if (injectBytes(output, (payload_loc[0] * BLOCK_SIZE) + STAGE1_EXTRAS_PARAMARR + (i * 0xC), s1load_params, 0xC))
                    return -1;
            }
        }
        if (appendBytes(output, full_va, ADDCONT_SIZE))
            return -1;
    }

    free(full_va);

    printf("all done!\n");
}