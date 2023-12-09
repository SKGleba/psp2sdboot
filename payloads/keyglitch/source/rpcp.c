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

int try_timed_xg(void) {
    maika_s* maika = (maika_s*)MAIKA_OFFSET;

    memset(0x4c000, 0, 0x400);
    crypto_bigmacDefaultCmd(true, 0x4c200, 0x4c300, 0x20, CRYPTO_BIGMAC_FUNC_FLAG_KEYSIZE_256 | CRYPTO_BIGMAC_FUNC_AES_CBC_ENC, 0x213, 0x4c220, 0);

    memset(0x4c220, 0, 0x20);
    maika->bigmac_ctrl.channel[true].dst = 0x4c320;

    gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

    maika->bigmac_ctrl.channel[true].trigger = 1;

    gpio_port_clear(0, GPIO_PORT_GAMECARD_LED);

    while ((maika->bigmac_ctrl.channel[true].res) & 1) {};
    if (memcmp(0x4c300, 0x4c320, 0x20)) {
        printf("\nFAILED %X\n", maika->bigmac_ctrl.channel[true].status);
        hexdump(0x4c300, 0x20, false);
        hexdump(0x4c320, 0x20, false);
        while (1) {};
    } else
        print("\nOK\n");
}

__attribute__((section(".text.rpcp")))
int rpcp(uint32_t arg0, uint32_t arg1, void* extra_data) {
    printf("\n[RPCP] hello world (%X, %X, %X)\n", arg0, arg1, (uint32_t)extra_data);

    int ret = -1;

    {

        vp 0xe3103040 = 0x10002;
        
        gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);

        while (1) {
            delay(0x80);
            try_timed_xg();
        }
        
    }

    print("[RPCP] byee\n\n");
    return ret;
}