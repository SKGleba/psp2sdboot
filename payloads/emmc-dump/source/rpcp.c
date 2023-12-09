#include "../../bob/include/types.h"

#include "../../bob/include/defs.h"
#include "../../bob/include/uart.h"
#include "../../bob/include/debug.h"
#include "../../bob/include/utils.h"
#include "../../bob/include/clib.h"
#include "../../bob/include/maika.h"
#include "../../bob/include/crypto.h"
#include "../../bob/include/spi.h"
#include "../../bob/include/perv.h"
#include "../../bob/include/gpio.h"
#include "../../bob/include/i2c.h"
#include "../../bob/include/paddr.h"
#include "../../bob/include/ernie.h"

#include "rpcp.h"

#include "include/brom1k.h"

#define TEST_COUNT 1

#define COPY_BUFFER_ADDR MAIN_DRAM_OFFSET
#define COPY_BUFFER_BLKSIZE 0x8000
#define TARGET_OFFSET_BLOCK 0x100

void prepare_emmc_regs(void) {
    vp 0xe3100124 |= 1;
    vp 0xe3101190 = 1;
    vp 0xe31020a0 = 1;
    while (!(vp 0xe31020a0)) {};
    vp 0xe31010a0 = 0;
    while ((vp 0xe31010a0)) {};
    vp 0xe3101190 = 0;
    while ((vp 0xe3101190)) {};
    vp 0xE0030024 = 0x1c0f020f;
    vp 0xe0070008 = 0x020e020f;
    vp 0xe0070000 = 1;
}

void prepare_sd_regs(void) {
    vp 0xe31020a4 = 1;
    while (!(vp 0xe31020a4)) {};
    vp 0xe31010a4 = 0;
    while ((vp 0xe31010a4)) {};
}

int prepare_emmc(uint32_t* ctx) {
    memset(BROM_RD_START, 0, BROM_RD_SIZE);
    ctx[0] = 0;
    ctx[1] = 0;
    int ret = brom_init_storages(0, (((vp 0xE0064060) >> 0x10) & 1) == 0);
    if (ret >= 0)
        ret = brom_init_mmc(0, ctx);
    return ret;
}

int prepare_sd(uint32_t* ctx) {
    memset(BROM_RD_START, 0, BROM_RD_SIZE);
    ctx[0] = 0;
    ctx[1] = 0;
    int ret = brom_init_storages(1, (((vp 0xE0064060) >> 0x11) & 1) == 0);
    if (ret >= 0)
        ret = brom_init_sd(1, ctx);
    return ret;
}

void set_sd_op_mode(bool write, bool notif) {
    if (notif)
        printf("[RPCP] set sd op mode: %s\n", write ? "WRITE" : "READ");
    if (write) {
        vp BROM_SDREAD_S_ARG0 = BROM_SDREAD_S_ARG0_WP; // single read arg0 0x11 -> 0x18
        vp BROM_SDREAD_S_ARG3 = BROM_SDREAD_S_ARG3_WP; // single read arg3 &0x100 -> &0x200
        vp BROM_SDREAD_M_ARG3 = BROM_SDREAD_M_ARG3_WP; // multi read arg3 &0x100 -> &0x200
        vp BROM_SDREAD_M_ARG0 = BROM_SDREAD_M_ARG0_WP; // multi read arg0 0x12 -> 0x19
        vp BROM_SDREAD_M_CHK1 = BROM_SDREAD_M_CHK1_WP; // check mulit read (0x12) -> check multi write (0x19)
    } else {
        vp BROM_SDREAD_S_ARG0 = BROM_SDREAD_S_ARG0_RP;
        vp BROM_SDREAD_S_ARG3 = BROM_SDREAD_S_ARG3_RP;
        vp BROM_SDREAD_M_ARG3 = BROM_SDREAD_M_ARG3_RP;
        vp BROM_SDREAD_M_ARG0 = BROM_SDREAD_M_ARG0_RP;
        vp BROM_SDREAD_M_CHK1 = BROM_SDREAD_M_CHK1_RP;
    }
}

void set_mmc_op_mode(bool write, bool notif) {
    if (notif)
        printf("[RPCP] set emmc op mode: %s\n", write ? "WRITE" : "READ");
    if (write) {
        vp BROM_MMCREAD_S_ARG3 = BROM_MMCREAD_S_ARG3_WP; // read arg3 &0x100 -> &0x200
        vp BROM_MMCREAD_CALC_ARG0 = BROM_MMCREAD_CALC_ARG0_WP; // read arg0 math 0x11 -> 0x18
        vp BROM_MMCREAD_M_CHK1 = BROM_MMCREAD_M_CHK1_WP; // check multi read (0x12) -> check multi write (0x19)
        vp BROM_MMCREAD_M_CHK2 = BROM_MMCREAD_M_CHK2_WP; // check2 multi read (0x12) -> check2 multi write (0x19)
    } else {
        vp BROM_MMCREAD_S_ARG3 = BROM_MMCREAD_S_ARG3_RP;
        vp BROM_MMCREAD_CALC_ARG0 = BROM_MMCREAD_CALC_ARG0_RP;
        vp BROM_MMCREAD_M_CHK1 = BROM_MMCREAD_M_CHK1_RP;
        vp BROM_MMCREAD_M_CHK2 = BROM_MMCREAD_M_CHK2_RP;
    }
}

int read_sd(uint32_t sector_off, void* dst, uint32_t sector_count) {
    uint32_t ctx[2];
    int ret = 0;
    printf("[RPCP] read_sd(%X, %X, %X)\n", sector_off, (uint32_t)dst, sector_count);
    ret = prepare_sd(ctx);
    if (ret < 0) {
        printf("[RPCP] read_sd: prepare sd failed: %X\n", ret);
        return ret;
    }
    set_sd_op_mode(false, false);
    ret = brom_read_sector_sd(ctx[0], sector_off, dst, sector_count);
    if (ret < 0)
        printf("[RPCP] read_sd: read failed: %X\n", ret);
    return ret;
}

int write_sd(uint32_t sector_off, void* dst, uint32_t sector_count) {
    uint32_t ctx[2];
    int ret = 0;
    printf("[RPCP] write_sd(%X, %X, %X)\n", sector_off, (uint32_t)dst, sector_count);
    ret = prepare_sd(ctx);
    if (ret < 0) {
        printf("[RPCP] write_sd: prepare sd failed: %X\n", ret);
        return ret;
    }
    set_sd_op_mode(true, false);
    ret = brom_read_sector_sd(ctx[0], sector_off, dst, sector_count);
    if (ret < 0)
        printf("[RPCP] write_sd: write failed: %X\n", ret);
    set_sd_op_mode(false, false);
    return ret;
}

int read_emmc(uint32_t sector_off, void* dst, uint32_t sector_count) {
    uint32_t ctx[2];
    int ret = 0;
    printf("[RPCP] read_emmc(%X, %X, %X)\n", sector_off, (uint32_t)dst, sector_count);
    ret = prepare_emmc(ctx);
    if (ret < 0) {
        printf("[RPCP] read_emmc: prepare emmc failed: %X\n", ret);
        return ret;
    }
    set_mmc_op_mode(false, false);
    ret = brom_read_sector_mmc(ctx[0], sector_off, dst, sector_count);
    if (ret < 0)
        printf("[RPCP] read_emmc: read failed: %X\n", ret);
    return ret;
}

int write_emmc(uint32_t sector_off, void* dst, uint32_t sector_count) {
    uint32_t ctx[2];
    int ret = 0;
    printf("[RPCP] write_emmc(%X, %X, %X)\n", sector_off, (uint32_t)dst, sector_count);
    ret = prepare_emmc(ctx);
    if (ret < 0) {
        printf("[RPCP] write_emmc: prepare emmc failed: %X\n", ret);
        return ret;
    }
    set_mmc_op_mode(true, false);
    ret = brom_read_sector_mmc(ctx[0], sector_off, dst, sector_count);
    if (ret < 0)
        printf("[RPCP] write_emmc: write failed: %X\n", ret);
    set_mmc_op_mode(false, false);
    return ret;
}

__attribute__((section(".text.rpcp")))
int rpcp(uint32_t arg0, uint32_t arg1, void* extra_data) {
    printf("\n[RPCP] hello world (%X, %X, %X)\n", arg0, arg1, (uint32_t)extra_data);

    int ret = 0;
    int testno = 0;
    int test_count = TEST_COUNT;
    //if (arg1)
        //test_count = arg1;
do_tests:
    testno++;
    printf("[RPCP] test number %X\n", testno);

    {
        int ret = 0;

        printf("[RPCP] prep emmc regs\n");
        prepare_emmc_regs();

        printf("[RPCP] set storage modes to read\n");
        set_mmc_op_mode(false, true);
        set_sd_op_mode(false, true);

        printf("[RPCP] start dump\n");
        uint32_t blksize = arg1;
        if (!blksize)
            blksize = COPY_BUFFER_BLKSIZE;
        uint32_t copied = arg0;
        while ((copied + COPY_BUFFER_BLKSIZE) <= (arg0 + blksize)) { // first copy with full buffer
            printf("[RPCP] memset %X %X\n", COPY_BUFFER_ADDR, COPY_BUFFER_BLKSIZE * 0x200);
            memset(COPY_BUFFER_ADDR, 0, COPY_BUFFER_BLKSIZE * 0x200);

            ret = read_emmc(copied, COPY_BUFFER_ADDR, COPY_BUFFER_BLKSIZE);
            if (ret < 0)
                return -1;

            if (copied == 0 && !arg1) {
                blksize = vp(COPY_BUFFER_ADDR + 0x24);
                printf("EMMC size detected: %X\n", blksize);
            }

            ret = write_sd(TARGET_OFFSET_BLOCK + copied, COPY_BUFFER_ADDR, COPY_BUFFER_BLKSIZE);
            if (ret < 0)
                return -2;

            copied = copied + COPY_BUFFER_BLKSIZE;
        }

        if (copied < (arg0 + blksize) && ((arg0 + blksize) - copied) <= COPY_BUFFER_BLKSIZE) {
            printf("[RPCP] memset %X %X\n", COPY_BUFFER_ADDR, COPY_BUFFER_BLKSIZE * 0x200);
            memset(COPY_BUFFER_ADDR, 0, COPY_BUFFER_BLKSIZE * 0x200);

            printf("[RPCP] read farts: %X[%X]\n", copied, ((arg0 + blksize) - copied));

            ret = read_emmc(copied, COPY_BUFFER_ADDR, ((arg0 + blksize) - copied));
            if (ret < 0)
                return -3;

            ret = write_sd(TARGET_OFFSET_BLOCK + copied, COPY_BUFFER_ADDR, ((arg0 + blksize) - copied));
            if (ret < 0)
                return -4;

            copied = (arg0 + blksize);
        }

        printf("[RPCP] dump done\n");

        printf("[RPCP] set storage modes to read\n");
        set_mmc_op_mode(false, true);
        set_sd_op_mode(false, true);

        ret = 0;
    }

    if (testno < test_count)
        goto do_tests;

    print("[RPCP] bye\n\n");
    return ret;
}