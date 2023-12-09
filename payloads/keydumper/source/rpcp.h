#ifndef __RPCP_H__
#define __RPCP_H__
#include "../../bob/include/types.h"

struct _partial_s {
    uint8_t full[0x10];
    uint8_t four[0x10];
    uint8_t eight[0x10];
    uint8_t twelve[0x10];
};
typedef struct _partial_s partial_s;

struct _keybuf_s {
    partial_s slsk_key_emmc_partials;
    partial_s slsk_key_sd_partials;
    uint8_t smi_enc_key[2][0x20];
    uint8_t snvs_keys[3][0x20];
};
typedef struct _keybuf_s keybuf_s;

#endif