DEVICE=atmega16a
BAUD?=9600
SERIAL_DEV=ttyS0
AVRDUDE=avrdude -c avrispmkII -p m16 -e
CC=avr-gcc
SIZE=avr-size
OBJDUMP=avr-objdump
OBJCOPY=avr-objcopy
CFLAGS=-std=gnu99 -Os -DBAUD=$(BAUD) -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=$(DEVICE)
LDFLAGS=-Wl,--gc-sections -mmcu=$(DEVICE)

PROGRAM?=avr_uart_stdio

HEX=$(PROGRAM).hex

all: $(HEX) size

size:
	$(SIZE) -C --mcu=$(DEVICE) $(PROGRAM).elf

prog: $(PROGRAM).burned

erase:
	rm -f $(PROGRAM).burned
	$(AVRDUDE) -e

%.burned: $(HEX)
	$(AVRDUDE) -U flash:w:$*.hex
	date >$@

prun: prog size run

reprun: clean prun

run:
	screen -q -r $(SERIAL_DEV) || screen -S $(SERIAL_DEV) /dev/$(SERIAL_DEV) $(BAUD)

close:
	screen -X -S $(SERIAL_DEV) kill || true

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< >$@

%.elf: %.o
	$(CC) $< -o $@ -Wl,-Map="$*.map" $(LDFLAGS)

%.hex: %.elf %.lss
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user $< $@
	
clean: close
	rm -f $(PROGRAM).lss $(PROGRAM).hex $(PROGRAM).o $(PROGRAM).elf $(PROGRAM).map

.PRECIOUS: %.o %.lss
.PHONY: clean
