# stage1 for psp2sdboot
# allows for fast main payload iteration

.section .text.stage1

.global vectors_exceptions
.type	vectors_exceptions, @object
vectors_exceptions:
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
    jmp RESET
.size	vectors_exceptions, .-vectors_exceptions

.global sdread_args
.type	sdread_args, @object
.size   sdread_args, 12
sdread_args:
    .word 0x0,0x0,0x0

.global RESET
.type   RESET, @function
RESET:
    di
    movh $sp, 0x5
    or3	$sp, $sp, 0xfff0
# sadly we have to reinit the gcsd - more often than not the controller is fked bc of the glitch
    mov $1, 0x1
    mov $2, 0x0
    bsr brom_init_storages
    mov $1, 0x1
    mov $2, 0x0
    bsr brom_init_sd
    movu $7, extra_sdreads
1:
    lw $2, ($7)
    blti $2, 0, 2f
    lw $4, 0x8 ($7)
    lw $3, 0x4 ($7)
    movu $1, brom_sdif_gctx
    bsr brom_read_sector_sd
    add $7, 4
    bra 1b
2:
    movu $7, sdread_args
    lw $4, 0x8 ($7)
    lw $3, 0x4 ($7)
    lw $2, ($7)
    movu $1, brom_sdif_gctx
    stc $3, $lp
    jmp brom_read_sector_sd
.size	RESET, .-RESET

.global extra_sdreads
.type   extra_sdreads, @object
extra_sdreads:
    .word 0xffffffff, 0xffffffff, 0xffffffff


    
