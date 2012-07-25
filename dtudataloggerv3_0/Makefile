# Configuring target etc
TARGET=datalogger_skel
#TARGET=bandgap_measure
#TARGET=bandgap
#TARGET=deepsleep
MCU=atmega1284p
F_CPU=16000000
FORMAT=ihex
UPLOAD_RATE=115200
#UPLOAD_RATE=19200
#UPLOAD_RATE=57600
ARDUINO_DIR=/home/attila/Documents/arduino-1.0.1
ARDUINO_VARIANT=$(ARDUINO_DIR)/hardware/mighty-1284p/variants/standard/
AVR_BIN=/usr/bin/ # avr-gcc bin
DEBUG= # Debugging (stabs)
OPT=s  # Optimization
#AVRDUDE_BIN=$(ARDUINO_DIR)/hardware/tools/avrdude 
AVRDUDE_BIN=/usr/bin/avrdude
AVRDUDE_PROGRAMMER=arduino #stk500v1
AVRDUDE_PORT=/dev/ttyUSB0
#AVRDUDE_PORT=/dev/ttyACM3
#AVRDUDE_CONF=$(ARDUINO_DIR)/hardware/tools/avrdude.conf
AVRDUDE_CONF=/etc/avrdude.conf

PROJECT_SRC=datalogger_skel.cpp

# Do not edit below
# C/C++ headers etc
# /hardware/arduino/cores/arduino
ARDUINO_HEADERS=$(ARDUINO_DIR)/hardware/mighty-1284p/cores/standard
CDEFS=-DF_CPU=$(F_CPU) -DARDUINO=100
CXXDEFS=-DF_CPU=$(F_CPU) -DARDUINO=100
CINCS=-I$(ARDUINO_HEADERS) -I$(ARDUINO_VARIANT)
CXXINCS=-I$(ARDUINO_HEADERS) -I$(ARDUINO_VARIANT)
CSTANDARD=-std=gnu99
CDEBUG=-g$(DEBUG)
CWARN=-Wall -Wstrict-prototypes
#CTUNING=-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CTUNING=-ffunction-sections -fdata-sections
CXXTUNING=-fno-exceptions -ffunction-sections -fdata-sections 

CFLAGS=$(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CWARN) $(CSTANDARD) $(CEXTRA)
CXXFLAGS=$(CXXTUNING) $(CDEFS) $(CINCS) -O$(OPT)
LDFLAGS= 

ARDUINO_SRC= \
$(ARDUINO_HEADERS)/wiring_pulse.c \
$(ARDUINO_HEADERS)/wiring_analog.c \
$(ARDUINO_HEADERS)/wiring.c \
$(ARDUINO_HEADERS)/wiring_digital.c \
$(ARDUINO_HEADERS)/WInterrupts.c \
$(ARDUINO_HEADERS)/wiring_shift.c \

ARDUINO_CXX_SRC = \
$(ARDUINO_HEADERS)/Tone.cpp \
$(ARDUINO_HEADERS)/Print.cpp \
$(ARDUINO_HEADERS)/HardwareSerial.cpp \
$(ARDUINO_HEADERS)/CDC.cpp \
$(ARDUINO_HEADERS)/HID.cpp \
$(ARDUINO_HEADERS)/IPAddress.cpp \
$(ARDUINO_HEADERS)/new.cpp \
$(ARDUINO_HEADERS)/Stream.cpp \
$(ARDUINO_HEADERS)/USBCore.cpp \
$(ARDUINO_HEADERS)/WMath.cpp \
$(ARDUINO_HEADERS)/WString.cpp \
$(ARDUINO_HEADERS)/main.cpp \

#ARDUINO_CXX_SRC += $(shell find $(ARDUINO_DIR)/libraries/  -maxdepth 2 -mindepth 2 -type f -name *.cpp -exec /bin/echo -n " {}" \;)

#ARDUINO_CXX_SRC += $(shell find $(ARDUINO_DIR)/libraries/ -type f -name "*.cpp" -exec /bin/echo -n " {}" \;)
ARDUINO_SRC += $(shell find ./ -maxdepth 3 -mindepth 2 -type f -name "*.c" -exec /bin/echo -n " {}" \;)

ARDUINO_CXX_SRC += $(shell find ./ -maxdepth 2 -mindepth 2 -type f -name "*.cpp" -exec /bin/echo -n " {}" \;)

#CINCS += $(shell find $(ARDUINO_DIR)/libraries/ -type d -exec /bin/echo -n " -I{}" \;) 
#CINCS += $(shell find ./ -type d -exec /bin/echo -n " -I{}" \;)
CINCS += $(shell find ./ -type d -name '\.git*' -prune -o -type d -exec /bin/echo -n " -I{}" \;)

CXXSRC=$(ARDUINO_CXX_SRC) $(PROJECT_SRC)
SRC=$(ARDUINO_SRC)


# Programming with avrdude
# /home/atiti/Documents/arduino-1.0/hardware/tools/avrdude -C/home/atiti/Documents/arduino-1.0/hardware/tools/avrdude.conf -v -v -v -v -patmega1284p -carduino -P/dev/ttyUSB0 -b115200 -D -Uflash:w:/tmp/build5759509888866614491.tmp/simpletest.cpp.hex:i
AVRDUDE_WRITE_FLASH=-D -U flash:w:$(TARGET).hex:i
AVRDUDE_FLAGS=-C $(AVRDUDE_CONF) -v -v -v -p $(MCU) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) -b $(UPLOAD_RATE)

# Program settings
CC=avr-gcc
CXX=avr-g++
LD=avr-gcc
AR=avr-ar
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
NM=avr-nm
REMOVE=rm -f
MV=mv -f

# Define all object files
OBJ=$(SRC:.c=.o) $(CXXSRC:.cpp=.o) $(ASRC:.S=.o)
FINAL_OBJ=$(TARGET).o core.a
# Define all listing files
LST=$(ASRC:.S=.lst) $(CXXSRC:.cpp=.lst) $(SRC:.c=.lst)

# Combine flags
ALL_CFLAGS=-mmcu=$(MCU) -I. $(CFLAGS)
ALL_CXXFLAGS=-mmcu=$(MCU) -I. $(CXXFLAGS)
ALL_ASFLAGS=-mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

LDFLAGS = -O$(OPT) -lm -Wl,--gc-sections 

all: build

build: sizebefore elf hex sizeafter

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym

upload: $(TARGET).hex $(TARGET).eep
	$(AVRDUDE_BIN) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

	# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(HEXSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(HEXSIZE); echo; fi

COFFCONVERT=$(OBJCOPY) --debugging \
--change-section-address .data-0x800000 \
--change-section-address .bss-0x800000 \
--change-section-address .noinit-0x800000 \
--change-section-address .eeprom-0x810000

coff: $(TARGET).elf
	$(COFFCONVERT) -O coff-avr $(TARGET).elf $(TARGET).cof


extcoff: $(TARGET).elf
	$(COFFCONVERT) -O coff-ext-avr $(TARGET).elf $(TARGET).cof


.SUFFIXES: .elf .hex .eep .lss .sym

.elf.hex:
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.eep:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
.elf.lss:
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
.elf.sym:
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
#$(TARGET).elf: $(OBJ)
#	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

$(TARGET).elf: $(FINAL_OBJ)
	$(LD) $(LDFLAGS) -mmcu=$(MCU) -o $@ $(FINAL_OBJ)

core.a: $(OBJ)
	@for i in $(OBJ); do echo $(AR) rcs core.a $$i; $(AR) rcs core.a $$i; done

# Compile: create object files from C++ source files.
.cpp.o:
	$(CXX) -c $(ALL_CXXFLAGS) $< -o $@ 

# Compile: create object files from C source files.
.c.o:
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Compile: create assembler files from C source files.
.c.s:
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
.S.o:
	$(CC) -c $(ALL_ASFLAGS) $< -o $@



# Target: clean project.
clean:
	$(REMOVE) $(TARGET).hex $(TARGET).eep $(TARGET).cof $(TARGET).elf \
	$(TARGET).map $(TARGET).sym $(TARGET).lss $(FINAL_OBJ) \
	$(OBJ) $(LST) $(SRC:.c=.s) $(SRC:.c=.d) $(CXXSRC:.cpp=.s) $(CXXSRC:.cpp=.d)

monitor:
	screen -ah 5000 $(AVRDUDE_PORT) 57600

.PHONY:	all build elf hex eep lss sym program coff extcoff clean depend

