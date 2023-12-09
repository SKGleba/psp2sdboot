#ifndef __BROM1K_H__
#define __BROM1K_H__

#include "../../../source/include/types.h"

int brom_init_storages(int id, int someflag);
int brom_init_mmc(int id, void* ctx_out);
int brom_init_sd(int id, void* ctx_out);
int brom_read_sector_sd(int ctx, uint32_t sector, void* dst, uint32_t count);
int brom_read_sector_mmc(int ctx, uint32_t sector, void* dst, uint32_t count);


#define BROM_RD_START 0x0005eb28
#define BROM_RD_SIZE 0x14c8


/*
    SD read => SD write conversion
    32bit patches
*/
/* # single-read arg0
        0005e1a6 14  e4  0d  00   bnei       r4,0x1 ,0x0005e1c0
        0005e1aa 11  5c           mov        r12 ,0x11
*/
#define BROM_SDREAD_S_ARG0 0x5e204
#define BROM_SDREAD_S_ARG0_RP 0x5c11000d
#define BROM_SDREAD_S_ARG0_WP 0x5c18000d // 0x11 -> 0x18
/* # single-read arg3
        0005e1ac 01  cb  14  01   mov        r11 ,0x114
*/
#define BROM_SDREAD_S_ARG3 0x5e208
#define BROM_SDREAD_S_ARG3_RP 0x0114cb01
#define BROM_SDREAD_S_ARG3_WP 0x0214cb01 // &0x100 -> &0x200
/* # multi-read arg0
        0005e1c4 06  4c           sw         r12 ,0x4 (sp)
        0005e1c6 12  5c           mov        r12 ,0x12
*/
#define BROM_SDREAD_M_ARG0 0x5e220
#define BROM_SDREAD_M_ARG0_RP 0x5c124c06
#define BROM_SDREAD_M_ARG0_WP 0x5c194c06 // 0x12 -> 0x19
/* # multi-read arg3
        0005e1c0 01  cc  14  09   mov        r12 ,0x914
*/
#define BROM_SDREAD_M_ARG3 0x5e21c
#define BROM_SDREAD_M_ARG3_RP 0x0914cc01
#define BROM_SDREAD_M_ARG3_WP 0x0a14cc01 // &0x100 -> &0x200
/* # check s/m mode
        0005e1e8 0b  4c           lw         r12 ,0x8 (sp)
        0005e1ea 12  5b           mov        r11 ,0x12
*/
#define BROM_SDREAD_M_CHK1 0x5e244
#define BROM_SDREAD_M_CHK1_RP 0x5b124c0b
#define BROM_SDREAD_M_CHK1_WP 0x5b194c0b // (0x12) -> (0x19)


/*
    MMC read => MMC write conversion
    32bit patches
*/
/* # calculate arg0
        0005dbb0 0d  60           sltu3      r0, r0,0x1
        0005dbb2 44  60           add        r0,0x11
*/
#define BROM_MMCREAD_CALC_ARG0 0x5dc0c
#define BROM_MMCREAD_CALC_ARG0_RP 0x6044600d
#define BROM_MMCREAD_CALC_ARG0_WP 0x6060600d // 0x11 -> 0x18
/* # arg3
        0005dba6 01  cb  14  01   mov        r11 ,0x114
        0005dbaa 06  4b           sw         r11 ,0x4 (sp)
*/
#define BROM_MMCREAD_S_ARG3 0x5dc04
#define BROM_MMCREAD_S_ARG3_RP 0x4b060114
#define BROM_MMCREAD_S_ARG3_WP 0x4b060214 // &0x100 -> &0x200
/* # check s/m mode
        0005dbc8 2a  44           sw         r4 ,0x28 (sp)  # r4=nSectors
        0005dbca 12  5a           mov        r10 ,0x12
*/
#define BROM_MMCREAD_M_CHK1 0x5dc24
#define BROM_MMCREAD_M_CHK1_RP 0x5a12442a
#define BROM_MMCREAD_M_CHK1_WP 0x5a19442a // (0x12) -> (0x19)
/* # check2 s/m mode
        0005dbf4 0b  4c           lw         r12 ,0x8 (sp)
        0005dbf6 12  5b           mov        r11 ,0x12
*/
#define BROM_MMCREAD_M_CHK2 0x5dc50
#define BROM_MMCREAD_M_CHK2_RP 0x5b124c0b
#define BROM_MMCREAD_M_CHK2_WP 0x5b194c0b // (0x12) -> (0x19)

#endif