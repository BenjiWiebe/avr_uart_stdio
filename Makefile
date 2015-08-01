ELF=test_bufferedio.elf
BAUD?=9600
F_CPU=8000000
SERIAL_DEV=ttyS0
DEVICE=atmega16a
DUDE_DEVNAME=m16
DUDE_ISP=avrispmkII
BACKEND=poll

AVRDUDE=avrdude -c $(DUDE_ISP) -p $(DUDE_DEVNAME) -e
CC=avr-gcc
SIZE=avr-size
OBJDUMP=avr-objdump
OBJCOPY=avr-objcopy
CFLAGS=-std=gnu99 -Os -DBAUD=$(BAUD) -DF_CPU=$(F_CPU) -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=$(DEVICE)
LDFLAGS=-Wl,--gc-sections -mmcu=$(DEVICE)
SOURCES=uart_stdio_$(BACKEND).c example.c
OBJECTS=$(SOURCES:.c=.o)
HEX=$(ELF:.elf=.hex)


all: $(OBJECTS) $(HEX) size

size:
	$(SIZE) -C --mcu=$(DEVICE) $(ELF)

prog: $(HEX).burned

erase:
	rm -f $(PROGRAM).burned
	$(AVRDUDE) -e

$(HEX).burned: $(HEX)
	$(AVRDUDE) -U flash:w:$(HEX)
	date >$@

prun: prog size run

zero: clean erase close

reprun: clean prun

run:
	screen -q -r $(SERIAL_DEV) || screen -S $(SERIAL_DEV) /dev/$(SERIAL_DEV) $(BAUD)

close:
	screen -X -S $(SERIAL_DEV) kill || true

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< >$@

%.elf: $(OBJECTS)
	$(CC) $^ -o $@ -Wl,-Map="$*.map" $(LDFLAGS)

%.hex: %.elf %.lss
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user $< $@
	
clean: close
	rm -f *.lss *.hex *.o *.elf *.map *.burned Makefile.bak

.PRECIOUS: %.o %.lss
.PHONY: clean
