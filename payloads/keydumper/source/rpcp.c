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

static const unsigned char xbr_k8_bin[] = {
  // FILL from brom
};

static const unsigned char xbr_k9_bin[] = {
    // FILL from brom
};

static const unsigned char smi_enc_seed_1[32] =
{
    // FILL from SL
};

static const unsigned char smi_enc_iv_1[16] =
{
    // FILL from SL
};

static const unsigned char smi_enc_seed_2[32] =
{
    // FILL from SL
};

static const unsigned char smi_enc_iv_2[16] =
{
    // FILL from SL
};

static const unsigned char snvs_enc_key_material[3 * 0x20] = {
    // FILL from SL
};

// reused key material from legacy static keys??????
static const unsigned char snvs_enc_key_material_iv[0x10] = {
    // FILL from SL
};

void xbr128(int target_ks, int enc_ks, void* seed, partial_s* dst) {
    crypto_bigmacDefaultCmd(0, (uint32_t)seed, target_ks, 0x10, CRYPTO_BIGMAC_FUNC_AES_ECB_ENC | CRYPTO_BIGMAC_FUNC_FLAG_TARGETS_KS | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256, enc_ks, 0, 0);
    crypto_bigmacDefaultCmd(0, DEVNULL_OFFSET, (uint32_t)dst->full, 0x10, CRYPTO_BIGMAC_FUNC_AES_ECB_DEC | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_128, target_ks, 0, 0);
    
    crypto_bigmacDefaultCmd(0, (uint32_t)seed, target_ks, 0x10, CRYPTO_BIGMAC_FUNC_AES_ECB_ENC | CRYPTO_BIGMAC_FUNC_FLAG_TARGETS_KS | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256, enc_ks, 0, 0);
    crypto_bigmacDefaultCmd(0, DEVNULL_OFFSET, (uint32_t)dst->four, 0xC, CRYPTO_BIGMAC_FUNC_AES_ECB_DEC | CRYPTO_BIGMAC_FUNC_FLAG_USE_EXT_KEY | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_128, 0, 0, 0);
    
    crypto_bigmacDefaultCmd(0, (uint32_t)seed, target_ks, 0x10, CRYPTO_BIGMAC_FUNC_AES_ECB_ENC | CRYPTO_BIGMAC_FUNC_FLAG_TARGETS_KS | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256, enc_ks, 0, 0);
    crypto_bigmacDefaultCmd(0, DEVNULL_OFFSET, (uint32_t)dst->eight, 0x8, CRYPTO_BIGMAC_FUNC_AES_ECB_DEC | CRYPTO_BIGMAC_FUNC_FLAG_USE_EXT_KEY | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_128, 0, 0, 0);
    
    crypto_bigmacDefaultCmd(0, (uint32_t)seed, target_ks, 0x10, CRYPTO_BIGMAC_FUNC_AES_ECB_ENC | CRYPTO_BIGMAC_FUNC_FLAG_TARGETS_KS | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256, enc_ks, 0, 0);
    crypto_bigmacDefaultCmd(0, DEVNULL_OFFSET, (uint32_t)dst->twelve, 0x4, CRYPTO_BIGMAC_FUNC_AES_ECB_DEC | CRYPTO_BIGMAC_FUNC_FLAG_USE_EXT_KEY | CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_128, 0, 0, 0);
}

__attribute__((section(".text.rpcp")))
int rpcp(uint32_t arg0, uint32_t arg1, void* extra_data) {
    printf("\n[RPCP] hello world (%X, %X, %X)\n", arg0, arg1, (uint32_t)extra_data);

    int ret = -1;
    
    {
        keybuf_s* keybuf = (keybuf_s*)0x1f850000;
        memset(keybuf, 0, sizeof(keybuf_s));

        // get SLSK key partials
        xbr128(8, 0x206, (void*)xbr_k8_bin, &keybuf->slsk_key_emmc_partials);
        xbr128(9, 0x207, (void*)xbr_k9_bin, &keybuf->slsk_key_sd_partials);

        // get SMI keys
        crypto_bigmacDefaultCmd(false, smi_enc_seed_1, keybuf->smi_enc_key[0], 0x20, CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256 | CRYPTO_BIGMAC_FUNC_AES_CBC_ENC, 0x213, smi_enc_iv_1, 0);
        crypto_bigmacDefaultCmd(false, smi_enc_seed_2, keybuf->smi_enc_key[1], 0x20, CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256 | CRYPTO_BIGMAC_FUNC_AES_CBC_ENC, 0x213, smi_enc_iv_2, 0);

        // get SNVS keys
        crypto_bigmacDefaultCmd(false, snvs_enc_key_material, keybuf->snvs_keys, 0x60, CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256 | CRYPTO_BIGMAC_FUNC_AES_CBC_ENC, 0x216, snvs_enc_key_material_iv, 0);

        // print out keys
        printf("SLSK key (EMMC) partials:\n");
        hexdump((uint32_t)&keybuf->slsk_key_emmc_partials, sizeof(partial_s), false);
        printf("SLSK key (SD) partials:\n");
        hexdump((uint32_t)&keybuf->slsk_key_sd_partials, sizeof(partial_s), false);
        printf("SMI encryption key (outer):\n");
        hexdump((uint32_t)keybuf->smi_enc_key[0], 0x20, false);
        printf("SMI encryption key (inner):\n");
        hexdump((uint32_t)keybuf->smi_enc_key[1], 0x20, false);
        printf("SNVS AES XTS tweak:\n");
        hexdump((uint32_t)keybuf->snvs_keys[0], 0x10, false);
        printf("SNVS AES XTS dec key:\n");
        hexdump((uint32_t)keybuf->snvs_keys[1], 0x10, false);
        printf("SNVS CMAC key:\n");
        hexdump((uint32_t)keybuf->snvs_keys[2], 0x10, false);
    }

    printf("[RPCP] byee %X\n\n", ret);
    return ret;
}