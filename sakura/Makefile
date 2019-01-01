
all: uip.bin

CC      = rx-elf-gcc
AR      = rx-elf-ar
OBJCOPY = rx-elf-objcopy
APPS    = webserver
CFLAGS  = -c -I../uip -I. -ffunction-sections -fdata-sections -fno-function-cse -fsigned-char -mlittle-endian-data -O2 -m64bit-doubles -mcpu=rx600 -Wall
LDFLAGS = -Wl,--gc-sections -nostartfiles -lc -lgcc

-include ../uip/Makefile.include

uip.bin: uip.elf
	$(OBJCOPY) -O binary $^ $@

uip.elf: main.o clock-arch.o crt0.o sakura_stubs.o sakura_uart.o sakura_timer.o sakura_clock.o sakura_eth.o phy.o apps.a uip.a
	$(CC) $(LDFLAGS) -T sakura_rx.ld $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -o $@

clock_arch.o: clock_arch.c
	$(CC) $(CFLAGS) $^ -o $@

crt0.o: sakura_crt0.S
	$(CC) $(CFLAGS) $^ -o $@

sakura_stubs.o: sakura_stubs.c
	$(CC) $(CFLAGS) $^ -o $@

sakura_uart.o: sakura_uart.c
	$(CC) $(CFLAGS) $^ -o $@

sakura_timer.o: sakura_timer.c
	$(CC) $(CFLAGS) $^ -o $@

sakura_clock.o: sakura_clock.c
	$(CC) $(CFLAGS) $^ -o $@

sakura_eth.o: sakura_eth.c
	$(CC) $(CFLAGS) $^ -o $@

phy.o: phy.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -fr *.o *~ *core uip $(OBJECTDIR) *.a *.bin *.elf

