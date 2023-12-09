@echo off

prompt vsh$$$S
DOSKEY bert=python bert.py $*
DOSKEY bob=python bob_rpc.py $*
DOSKEY misc=python misc.py $*
DOSKEY alice=python alice_rpc.py $*
DOSKEY teensy=python teensy_rpc.py $*
DOSKEY sdboot=python sdboot.py $*

DOSKEY info=python bert.py info $*
DOSKEY mode=python bert.py mode $*
DOSKEY unlock-1=python bert.py unlock-1 $*
DOSKEY lock-1=python bert.py lock-1 $*
DOSKEY power-off=python bert.py power-off $*
DOSKEY power-on=python bert.py power-on $*
DOSKEY power-fsm=python bert.py power-fsm $*
DOSKEY get-power=python bert.py get-power $*
DOSKEY handshake-0=python bert.py handshake-0 $*
DOSKEY handshake-1=python bert.py handshake-1 $*
DOSKEY handshake-E=python bert.py handshake-E $*
DOSKEY get-kr600=python bert.py get-kr600 $*
DOSKEY maika-0=python bert.py maika-0 $*
DOSKEY nvs-read=python bert.py nvs-read $*
DOSKEY nvs-read-range=python bert.py nvs-read-range $*
DOSKEY nvs-write=python bert.py nvs-write $*
DOSKEY confzz-read=python bert.py confzz-read $*
DOSKEY confzz-read-range=python bert.py confzz-read-range $*
DOSKEY confzz-write=python bert.py confzz-write $*
DOSKEY confzz-rw=python bert.py confzz-rw $*
DOSKEY confzz-ro=python bert.py confzz-ro $*
DOSKEY confzz-apply=python bert.py confzz-apply $*
DOSKEY invs-read-id=python bert.py invs-read-id $*
DOSKEY invs-read=python bert.py invs-read $*
DOSKEY invs-read-range=python bert.py invs-read-range $*
DOSKEY wipe-nvs=python bert.py wipe-nvs $*
DOSKEY reset=python bert.py reset $*
DOSKEY reset-hard=python bert.py reset-hard $*
DOSKEY kill=python bert.py kill $*
DOSKEY reset-bic=python bert.py reset-bic $*
DOSKEY unlock-4=python bert.py unlock-4 $*
DOSKEY lock-4=python bert.py lock-4 $*
DOSKEY unlock-qa=python bert.py unlock-qa $*
DOSKEY unlock-nvs=python bert.py unlock-nvs $*
DOSKEY unlock-sdboot=python bert.py unlock-sdboot $*
DOSKEY unlock-all=python bert.py unlock-all $*
DOSKEY shbuf-read=python bert.py shbuf-read $*
DOSKEY shbuf-write=python bert.py shbuf-write $*

if %1.==alice. (
    echo load rpc env for alice
    DOSKEY read32=python alice_rpc.py read32 $*
    DOSKEY write32=python alice_rpc.py write32 $*
    DOSKEY memset=python alice_rpc.py memset $*
    DOSKEY memset32=python alice_rpc.py memset32 $*
    DOSKEY memcpy=python alice_rpc.py memcpy $*
    DOSKEY delay=python alice_rpc.py delay $*
    DOSKEY hexdump=python alice_rpc.py hexdump $*
    DOSKEY load_sk=python alice_rpc.py load_sk $*
    DOSKEY bob_read32=python alice_rpc.py bob_read32 $*
    DOSKEY bob_write32=python alice_rpc.py bob_write32 $*
    DOSKEY copyto=python alice_rpc.py copyto $*
    DOSKEY copyfrom=python alice_rpc.py copyfrom $*
    DOSKEY exec=python alice_rpc.py exec $*
    DOSKEY file_send=python alice_rpc.py file_send $*
    DOSKEY file_dump=python alice_rpc.py file_dump $*
    DOSKEY file_append=python alice_rpc.py file_append $*
) else (
    echo load rpc env for bob
    DOSKEY read32=python bob_rpc.py read32 $*
    DOSKEY write32=python bob_rpc.py write32 $*
    DOSKEY memset=python bob_rpc.py memset $*
    DOSKEY memset32=python bob_rpc.py memset32 $*
    DOSKEY memcpy=python bob_rpc.py memcpy $*
    DOSKEY delay=python bob_rpc.py delay $*
    DOSKEY hexdump=python bob_rpc.py hexdump $*
    DOSKEY arm_reset=python bob_rpc.py arm_reset $*
    DOSKEY set_xctable=python bob_rpc.py set_xctable $*
    DOSKEY set_ints=python bob_rpc.py set_ints $*
    DOSKEY alice_rpc=python bob_rpc.py alice_rpc $*
    DOSKEY alice_task_status=python bob_rpc.py alice_task_status $*
    DOSKEY copyto=python bob_rpc.py copyto $*
    DOSKEY copyfrom=python bob_rpc.py copyfrom $*
    DOSKEY exec=python bob_rpc.py exec $*
    DOSKEY execbig=python bob_rpc.py execbig $*
    DOSKEY file_send=python bob_rpc.py file_send $*
    DOSKEY file_dump=python bob_rpc.py file_dump $*
    DOSKEY file_append=python bob_rpc.py file_append $*
    DOSKEY alice_schedule_task=python bob_rpc.py alice_schedule_task $*
)

DOSKEY test=python misc.py test $*
DOSKEY maika=python misc.py maika $*
DOSKEY rmem=python misc.py rmem $*
DOSKEY wmem=python misc.py wmem $*
DOSKEY xmem=python misc.py xmem $*
DOSKEY enol=python misc.py enol $*
DOSKEY disol=python misc.py disol $*