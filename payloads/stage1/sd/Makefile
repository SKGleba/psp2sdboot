all: stage1_4v2.bin
stage1.o: stage1.s
	mep-elf-gcc -c -o $@ $< -ml -Wall

stage1_4v2.elf: stage1.o
	mep-elf-gcc -o $@ $^ -Wl,-T cfg_4v2.x -nodefaultlibs -nostdlib -nostartfiles

%.bin: %.elf
	mep-elf-objcopy -O binary $< $@
	truncate -s 1536 $@

clean:
	-rm stage1.o
	-rm stage1*.elf
	-rm stage1*.bin