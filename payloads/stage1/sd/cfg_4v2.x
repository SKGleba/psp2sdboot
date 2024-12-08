SECTIONS
{
    . = 0x00040000;
    .text : { *(.text.stage1) }
	brom_read_sector_sd = 0x5e194;
    brom_sdif_sctx = 0x5eda0;
    brom_sdif_gctx = 0x5ea80;
    brom_init_storages = 0x5cf88;
    brom_init_sd = 0x5e252;
}
